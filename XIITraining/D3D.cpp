#include "D3D.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "DescriptorAllocator.h"
#include "CommittedAllocator.h"
#include "PlacedAllocator.h"
#include "MeshGenerator.h"

void D3D::Init()
{
#if (DEBUG) || (_DEBUG)
	// 启用 D3D12 调试层
	D3D12GetDebugInterface(IID_PPV_ARGS(&g_debugController));
	g_debugController->EnableDebugLayer();
#endif

	// 创建DX12设备
	CreateDevice();

	// 创建命令队列、命令分配器、命令列表
	CreateCommandObjects();

	// 创建交换链
	CreateSwapChain();

	// 创建描述符分配器
	g_pDescriptorAllocator = new DescriptorAllocator(g_pDevice.Get());
	
	// 创建 CBuffer 分配器
	g_pCBufferAllocator = new CommittedAllocator(g_pDevice.Get());

	// 纹理分配器
	g_pTextureAllocator = new PlacedAllocator(g_pDevice.Get());

	// Mesh 生成类，负责生成并管理各种 Mesh
	MeshGenerator::GetInstance()->Init();

	// 分配 CBPerFrame（现在里面只有相机的VP矩阵）
	AllocCBufferPerFrame();

	// 创建实际应用到Mesh上的纹理资源
	m_pTextureBox = new Texture();
	m_pTextureBox->Load("D:\\NixAssets\\rustediron2\\albedo.png", "My Texture");
	m_pTextureBox->AddSRV(TextureType_2D);

	m_pTextureCubeMap = new Texture();
	m_pTextureCubeMap->Load("D:\\NixAssets\\HDR\\ballroom_4k.dds", "My Cube");
	m_pTextureCubeMap->AddSRV(TextureType_Cube);

	// 创建材质，指定纹理，手动编译。
	auto pMaterialBox = new Material();
	pMaterialBox->Load(".\\Color.fx", "VSMain", "PSMain", "vs_5_0", "ps_5_0");
	pMaterialBox->SetTexture(m_pTextureBox);
	pMaterialBox->Reprofile();

	auto pMaterialCubeMap = new Material();
	pMaterialCubeMap->Load(".\\Sky.fx", "VSMain", "PSMain", "vs_5_0", "ps_5_0");
	pMaterialCubeMap->SetTexture(m_pTextureCubeMap);
	pMaterialCubeMap->Reprofile();

	m_pMaterials.push_back(pMaterialBox);
	m_pMaterials.push_back(pMaterialCubeMap);

	MeshGenerator::GetInstance()->CreateBox("Box", 1.0f);

	// 建模型
	m_pMesh = new Mesh();
	m_pMesh->Init("Box");
	m_pMesh->SetScale(1.0f, 1.0f, 1.0f);
	m_pMesh->SetRotate(true);

	m_pMeshCube = new Mesh();
	m_pMeshCube->Init("Box");
	m_pMeshCube->SetScale(100.0f, 100.0f, 100.0f);
	//m_pMeshCube->SetScale(0.1f, 0.1f, 0.1f);
	m_pMeshCube->SetRotate(true);

	// 给模型绑材质
	m_pMesh->SetMaterial(pMaterialBox);
	m_pMeshCube->SetMaterial(pMaterialCubeMap);

	// 初始化流程结束后，默认关闭命令列表
	g_pCommandList->Close();

	ID3D12CommandList* pCmdLists[] = { g_pCommandList.Get() };
	g_pCommandQueue->ExecuteCommandLists(1, pCmdLists);

	// 初始化栅栏值均为0。
	m_currFenceIdx = 0;
	m_lastFenceIdx.Reset(0);

	FlushCommandQueue();
}

void D3D::CreateDevice()
{
	// 创建 DXGI 工厂
	HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_pDXGIFactory));

	// 从 DXGI 工厂中 枚举第一个适配器（默认的显卡）
	ComPtr<IDXGIAdapter1> pAdapter;
	m_pDXGIFactory->EnumAdapters1(0, &pAdapter);

	// 转换成 IDXGIAdapter4。
	// IDXGIAdapter 是最初始版本，不推荐使用；
	// IDXGIAdapter1：允许访问显卡的供应商ID、设备ID、子系统ID等。
	// IDXGIAdapter2：允许对显卡进行功耗管理；
	// IDXGIAdapter3：进一步增加了对显存使用情况的查询功能。
	// IDXGIAdapter4 提供了对 GPU 更高级别的控制和监视。特别是对硬件性能数据的访问。
	ComPtr<IDXGIAdapter4> pAdapter4;
	hr = pAdapter.As(&pAdapter4);

	// 创建 D3D12 设备
	// ID3D12Device 也分成若干版本，由于版本太多了，挑最主要的说：
	// 从 ID3D12Device4 开始，增加了对光线追踪功能的接口支持。
	// 越老的版本兼容性和稳定性越强。自己酌情选择。
	hr = D3D12CreateDevice(pAdapter4.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&g_pDevice));

	// 创建 Fence 用于同步 CPU 和 GPU 的操作。
	hr = g_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));

	// 获取 描述符堆 的 句柄增量大小。
	// 句柄增量大小：是 DX12 的新概念。在 DX12 中，使用 描述符堆 存储 描述符。
	// 目前 DX12 的最新版本，分成了四个类型的堆：RTV，DSV，Sampler，CBV/SRV/UAV。
	// 举个例子，假设我们有一个 DSV描述符堆，里面全是 DSV描述符。
	// 现在我们想要这个堆中的第3个描述符，于是首先需要调用下面的方法，获取 DSVDescriptorSize，例如 = 32
	// 那么该描述符在堆中的起始字节就是 32 * 3 = 96。
	// note：DX12中，一个描述符堆下的所有 描述符 类型一致。不会出现 DSV/RTV/XX... 混在一个堆里的情况。
	// 作为初学者，可以先不管它们的含义是什么。先把获取大小、字节偏移的概念理解就好。
	m_nRTVDescriptorSize = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_nDSVDescriptorSize = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_nSamplerDescriptorSize = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	m_nCBSRUAVDescriptorSize = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void D3D::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; 
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT hr;

	// 创建命令队列
	hr = g_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_pCommandQueue));

	for (int i = 0; i < FRAME_BUFFER_NUM; i++)
	{
		// 创建命令分配器
		hr = g_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_pCommandAllocator[i]));

		// 创建命令列表，并将其和 命令分配器 关联
		// 同时设定初始渲染管线状态 = nullptr（默认状态）
		hr = g_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_pCommandAllocator[i].Get(), nullptr, IID_PPV_ARGS(&g_pCommandList));
	}
}

void D3D::CreateSwapChain()
{
	HRESULT hr;

	// 获取窗口大小
	RECT rect;
	GetClientRect(g_hWnd, &rect);
	UINT width = rect.right - rect.left;
	UINT height = rect.bottom - rect.top;

	// 填充 SwapChain 描述结构体
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 8 位 RGBA
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60; // 刷新率
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.SampleDesc.Count = 1; // MSAA4x 禁用
	swapChainDesc.SampleDesc.Quality = 0; 
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 用于RT
	swapChainDesc.BufferCount = FRAME_BUFFER_NUM; // n缓冲
	swapChainDesc.OutputWindow = g_hWnd; // 窗口句柄
	swapChainDesc.Windowed = true; // 窗口模式
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 翻转模式
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // 允许全屏切换

	// 创建交换链 并且 转换成 IDXGISwapChain4
	ComPtr<IDXGISwapChain> pSwapChain;
	hr = m_pDXGIFactory->CreateSwapChain(g_pCommandQueue.Get(), &swapChainDesc, &pSwapChain);
	hr = pSwapChain.As(&m_pSwapChain);

	// 创建交换链专用的RTV/DSV描述符堆。
	CreateDescriptorHeapForSwapChain();
}

void D3D::CreateDescriptorHeapForSwapChain()
{
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVHeapDesc.NodeMask = 0;
	RTVHeapDesc.NumDescriptors = FRAME_BUFFER_NUM; // RTV描述符堆 内置n张描述符，对应了 SwapChain 的n缓冲。

	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVHeapDesc.NodeMask = 0;
	DSVHeapDesc.NumDescriptors = 1;

	g_pDevice->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_pRTVHeap));
	g_pDevice->CreateDescriptorHeap(&DSVHeapDesc, IID_PPV_ARGS(&m_pDSVHeap));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());

	// 创建交换链的n个RT
	for (int i = 0; i < FRAME_BUFFER_NUM; i++)
	{
		// 获取其中一个RT
		m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pSwapChainRT[i]));
		std::wstring wstr = std::wstring(L"SwapChainRT") + std::to_wstring(i);
		m_pSwapChainRT[i]->SetName(wstr.c_str());

		// 为其创建一个RTV描述符。
		// 如果熟悉DX11的话，其实View和Sampler，就是描述符。
		// 但在对应的RTV描述符堆创建具体的RTV描述符的时候，DX12 Device，并没有 "创建描述符" 这样的说法。
		// 而是保留了DX11的风格 叫做 "创建 View"/"创建Sampler"。
		// 但是，这和DX11本质上有区别。
		//		DX11：直接创建RTV本身
		//		DX12：在RTV描述符堆里，创建一个指向RTV的指针。
		g_pDevice->CreateRenderTargetView(m_pSwapChainRT[i].Get(), nullptr, rtvHandle); // 在 rtvHandle 对应的 RTV描述符堆里，创建一个RTV。
		rtvHandle.Offset(1, m_nRTVDescriptorSize);
	}

	// 创建深度Buffer资源的desc
	// 交换链的BackBuffer不需要创建这个，如前面的方法所述，创建交换链的BackBuffer时，使用专门的DXGI_SWAP_CHAIN_DESC
	//		（但是如果你想创建一个普通RT，还是要使用这里的 D3D12_RESOURCE_DESC 创建）
	D3D12_RESOURCE_DESC depthDesc;
	depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.MipLevels = 1;
	depthDesc.Width = m_width;
	depthDesc.Height = m_height;
	depthDesc.DepthOrArraySize = 1;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	depthDesc.Alignment = 0;

	// 在DX12中，创建纹理资源的时候，需要明确：这个资源将提交到什么样的堆里？
	// 这里所谓的堆，就是GPU中的若干显存块。在现代GPU中，显存块的类型包括：
	// 1. 默认堆：GPU 可访问，CPU 永远不可访问。
	// 2. 上传堆：这个堆中的资源始终在等待将它们从 CPU 上传到 GPU。
	// 3. 回读堆：这个堆中的资源始终在等待 CPU 读取它们。
	// 4. 自定义堆：（特性相对高级，萌新暂时不用看）
	D3D12_HEAP_PROPERTIES heapProps;
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;  // 指定堆类型为 1.默认堆
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  // CPU页面属性不适用于默认堆
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;  // 内存池偏好设置为未知，因为这是默认堆
	heapProps.CreationNodeMask = 0;  // 创建节点掩码，用于多GPU系统，0表示所有GPU节点
	heapProps.VisibleNodeMask = 0;   // 可见节点掩码，用于多GPU系统，0表示所有GPU节点


	// 创建深度Buffer资源本体
	CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0f, 0x00);
	g_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &depthDesc, D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&m_pDepthStencilBuffer));
	m_pDepthStencilBuffer->SetName(L"Depth/Stencil Buffer");

	// 创建DSV描述符。和前面创建RTV描述符同理。
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_pDSVHeap->GetCPUDescriptorHandleForHeapStart());
	g_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, dsvHandle);
}

void D3D::AllocCBufferPerFrame()
{
	for (int i = 0; i < FRAME_BUFFER_NUM; i++)
	{
		g_pCBufferAllocator->Alloc(g_cbPerFrame[i].DataByteSize(), ResourceType_Upload, g_cbPerFrame[i].GPUVirtualAddr, g_cbPerFrame[i].pageIndex, g_cbPerFrame[i].pageByteOffset);

		// 创建CBV
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = g_cbPerFrame[i].GPUVirtualAddr; // 常量缓冲区的GPU虚拟地址
		cbvDesc.SizeInBytes = (UINT)((g_cbPerFrame[i].DataByteSize() + 255) & ~255);

		// 分配描述符
		D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle;
		UINT nouse[2];
		if (g_pDescriptorAllocator->Alloc(DescriptorType_CBV, 1, nouse[0], nouse[1], cbvCpuHandle))
			g_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D::GetSwapChainBackBufferRTV()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart(), m_nRTVDescriptorSize, m_backBufferIndex);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D::GetSwapChainBackBufferDSV()
{
	return m_pDSVHeap->GetCPUDescriptorHandleForHeapStart();
}

ID3D12Resource* D3D::GetSwapChainBackBuffer() const
{
	return m_pSwapChainRT[m_backBufferIndex].Get();
}

void D3D::Prepare()
{
	m_backBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	for (auto& pMaterial : m_pMaterials)
	{
		// 获取这个材质使用的所有 non-shader-visible (cpu) 描述符
		const size_t* pDescriptors = pMaterial->GetViewsGroup();
		const size_t pDescriptorsSize = pMaterial->GetViewsGroupSize();

		// 将这些 描述符 追加到 shader-visible (gpu) 的描述符堆 
		UINT renderHeapOffset = g_pDescriptorAllocator->AppendToRenderHeap(pDescriptors, pDescriptorsSize);
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = g_pDescriptorAllocator->GetRenderHeap()->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += renderHeapOffset * g_pDescriptorAllocator->GetRenderHeapDescriptorByteSize();

		pMaterial->SetGPUHandle(gpuHandle);
	}
}

void D3D::Render()
{
	auto& pCmdAllocator = g_pCommandAllocator[m_backBufferIndex];

	HRESULT hr;
	hr = pCmdAllocator->Reset();

	hr = g_pCommandList->Reset(pCmdAllocator.Get(), nullptr);

	// 设置视口
	CD3DX12_VIEWPORT vp(0.0f, 0.0f, (float)m_width, (float)m_height);
	g_pCommandList->RSSetViewports(1, &vp);

	// 创建一个与视口大小相同的剪切矩形
	CD3DX12_RECT scissorRect(0, 0, (LONG)m_width, (LONG)m_height);
	g_pCommandList->RSSetScissorRects(1, &scissorRect);

	// 设置当前帧 backBuffer 的资源状态为 RENDERTARGET。
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(GetSwapChainBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	g_pCommandList->ResourceBarrier(1, &barrier);

	// 设置当前帧 depthBuffer 的资源状态为 DEPTHWRITE。
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	g_pCommandList->ResourceBarrier(1, &barrier);

	auto currSwapChainRTV = GetSwapChainBackBufferRTV();
	auto currSwapChainDSV = GetSwapChainBackBufferDSV();
	g_pCommandList->ClearRenderTargetView(currSwapChainRTV, m_backBufferIndex ? DirectX::Colors::LightSteelBlue : DirectX::Colors::LightSteelBlue, 0, nullptr);
	g_pCommandList->ClearDepthStencilView(currSwapChainDSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	g_pCommandList->OMSetRenderTargets(1, &currSwapChainRTV, true, &currSwapChainDSV);

	{
		g_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D12DescriptorHeap* pRenderHeap = g_pDescriptorAllocator->GetRenderHeap();
		ID3D12DescriptorHeap* ppHeaps[] = { pRenderHeap };
		g_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		for (auto& pMat : m_pMaterials)
		{
			pMat->Render(m_backBufferIndex);
		}
	}

	// Clear，SetRT执行完，将资源状态重置回 PRESENT
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(GetSwapChainBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	g_pCommandList->ResourceBarrier(1, &barrier);

	// Clear，SetRT执行完，将资源状态重置回 COMMON
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON);
	g_pCommandList->ResourceBarrier(1, &barrier);

	g_pCommandList->Close();

	ID3D12CommandList* pCmdLists[] = { g_pCommandList.Get() };
	g_pCommandQueue->ExecuteCommandLists(1, pCmdLists);

	m_pSwapChain->Present(0, 0);

	FlushCommandQueue();
}

void D3D::FlushCommandQueue()
{
	m_currFenceIdx++;

	// 通过 Signal，告知GPU：在 Queue 执行完毕后，将值设置到 m_currFenceIdx
	g_pCommandQueue->Signal(m_pFence.Get(), m_currFenceIdx);

	// 一旦 m_currFenceIdx = x，就代表 CPU 接下来将处理 x 帧。
	// 一旦 m_pFence->GetCompletedValue() = x，就代表 GPU 接下来将处理 x 帧。
	// 所以，若 CPU - GPU = t，说明最多可能有同时 t+1 帧的资源正在被处理。

	// 换言之，一旦 CPU - GPU > N - 1，说明CPU-GPU之间，已经积累了 N 帧的数据差。
	// 此时如果继续让CPU积累下去，就会超过n缓冲的n帧限制，造成未定义行为。此时必须通过事件进行等待。
	if (m_currFenceIdx - m_pFence->GetCompletedValue() > FRAME_BUFFER_NUM - 1)
	{
		//printf("%lld, %lld\n", m_currFenceIdx, m_pFence->GetCompletedValue());

		// 创建一个 Windows 事件 fenceEvent。
		HANDLE fenceEvent = CreateEvent(nullptr, false, false, nullptr);

		// 通过下面的方法告知 GPU 值达到 m_currFenceIdx 事件时，向 CPU 推送一个 fenceEvent。
		// “等待GPU执行命令，直到CPU和GPU之间差N-1帧，才继续让CPU执行”
		m_pFence->SetEventOnCompletion(m_currFenceIdx - FRAME_BUFFER_NUM + 1, fenceEvent);

		// 让 Windows 持续等待这个 fence。
		// 换言之，CPU 这边将持续等待，直到 GPU 那边的值确实的变化到 m_currFenceIdx - FRAME_BUFFER_NUM + 1。
		WaitForSingleObject(fenceEvent, INFINITE);

		// 当执行到这里，说明 GPU 的值已经确实的变化到 m_currFenceIdx - FRAME_BUFFER_NUM + 1，这时就可以将这个事件关闭掉了。
		// 这样也就完成了一次 CPU 和 GPU 之间的通信同步。
		CloseHandle(fenceEvent);
	}
}

void D3D::Update()
{
	auto& currCBPerFrameData = g_cbPerFrame[m_backBufferIndex];

	// 暂时先使用固定位置的相机
	currCBPerFrameData.data.m_view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, -4.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)).Transpose();
	currCBPerFrameData.data.m_proj = Matrix::CreatePerspectiveFieldOfView(60.0f / 180.0f * 3.1415926f, (float)m_width / (float)m_height, 0.01f, 300.0f).Transpose();

	g_pCBufferAllocator->UpdateData(&currCBPerFrameData.data, currCBPerFrameData.DataByteSize(), currCBPerFrameData.pageIndex, currCBPerFrameData.pageByteOffset);

	for (auto& pMat : m_pMaterials)
	{
		for (auto& pMesh : pMat->GetSubMeshes())
		{
			pMesh->Update(m_backBufferIndex);
		}
	}
}

void D3D::Release()
{
	for (auto pMat : m_pMaterials)
	{
		for (auto pMesh : pMat->GetSubMeshes())
		{
			delete pMesh;
			pMesh = nullptr;
		}

		delete pMat;
		pMat = nullptr;
	}

	if (m_pTextureBox) delete m_pTextureBox;
	if (m_pTextureCubeMap) delete m_pTextureCubeMap;
}
