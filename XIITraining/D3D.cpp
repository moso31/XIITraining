#include "D3D.h"
#include "Mesh.h"

void D3D::Init()
{
	// 启用 D3D12 调试层
#if defined(DEBUG) || defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	debugController->EnableDebugLayer();
#endif

	HRESULT hr;

	// 创建 DXGI 工厂
	hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_pDXGIFactory));

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
	hr = D3D12CreateDevice(pAdapter4.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&g_pDevice));

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

	// 创建命令队列、命令分配器、命令列表
	CreateCommandObjects();

	// 创建交换链
	CreateSwapChain();

	CreateRootSignature();

	// 创建描述符堆
	CreateDescriptorHeap();

	// 创建实际应用到Mesh上的纹理资源
	CreateMyTexture();
	CreateCubeMap();

	CreateGlobalConstantBuffers();

	CreateShaderAndPSO();

	m_pMesh = new Mesh();
	m_pMesh->InitBox();
	m_pMesh->SetScale(1.0f, 1.0f, 1.0f);
	m_pMesh->SetRotate(true);

	m_pMeshCube = new Mesh();
	m_pMeshCube->InitBox();
	m_pMeshCube->SetScale(100.0f, 100.0f, 100.0f);
	//m_pMeshCube->SetScale(0.1f, 0.1f, 0.1f);
	m_pMeshCube->SetRotate(false);

	// 暂时先使用固定的相机参数
	g_cbObjectData.m_view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, -4.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)).Transpose();
	g_cbObjectData.m_proj = Matrix::CreatePerspectiveFieldOfView(60.0f / 180.0f * 3.1415926f, (float)m_width / (float)m_height, 0.01f, 300.0f).Transpose();
	
	// 初始化流程结束后，默认关闭命令列表
	// 虽然并非官方规定，但这是一个常见实践做法。
	// 初始关闭命令列表，需要用这个列表的时候，使用 Reset 重新打开它，然后再记录新的命令。
	// 这样可以避免 CommandList 在后续操作中被其它逻辑掺入命令，导致渲染错误。
	// 要跳过Close，除非你对这个命令列表之后的调用逻辑非常明确。但是，何必呢？
	g_pCommandList->Close();

	ID3D12CommandList* pCmdLists[] = { g_pCommandList.Get() };
	g_pCommandQueue->ExecuteCommandLists(1, pCmdLists);

	FlushCommandQueue();
}

void D3D::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; 
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT hr;

	// 创建命令队列
	hr = g_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_pCommandQueue));

	// 创建命令分配器
	hr = g_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_pCommandAllocator));

	// 创建命令列表，并将其和 命令分配器 关联
	// 同时设定初始渲染管线状态 = nullptr（默认状态）
	hr = g_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&g_pCommandList));
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
	swapChainDesc.BufferCount = m_swapChainBufferCount; // 双缓冲
	swapChainDesc.OutputWindow = g_hWnd; // 窗口句柄
	swapChainDesc.Windowed = true; // 窗口模式
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 翻转模式
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // 允许全屏切换

	// 创建交换链 并且 转换成 IDXGISwapChain4
	ComPtr<IDXGISwapChain> pSwapChain;
	hr = m_pDXGIFactory->CreateSwapChain(g_pCommandQueue.Get(), &swapChainDesc, &pSwapChain);
	hr = pSwapChain.As(&m_pSwapChain);
}

void D3D::CreateDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVHeapDesc.NodeMask = 0;
	RTVHeapDesc.NumDescriptors = m_swapChainBufferCount; // RTV描述符堆 内置两张描述符，对应了 SwapChain 的双缓冲。

	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVHeapDesc.NodeMask = 0;
	DSVHeapDesc.NumDescriptors = 1;

	g_pDevice->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_pRTVHeap));
	g_pDevice->CreateDescriptorHeap(&DSVHeapDesc, IID_PPV_ARGS(&m_pDSVHeap));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());

	// 创建交换链的两个RT
	for (int i = 0; i < m_swapChainBufferCount; i++)
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
	// 当然，如上面所述，交换链的RT也不需要创建这个。
	// 下面的参数中，没有提到的有两个：
	// D3D12_HEAP_FLAG_NONE：先不用管，让它等于none就行。
	// 
	// D3D12_RESOURCE_STATE_COMMON：资源状态。COMMON表示这是资源转换规则中的初始状态。
	// 资源状态：可以理解成 DX12 的新概念。（尽管本质上 DX11 也有资源转换，但都封装在驱动层，DX11 API调用者通常不需要考虑这些。）
	//		以这里的 深度buffer 为例，最开始的状态是 D3D12_RESOURCE_STATE_COMMON，表示这个资源是可用的，但是还没有被任何流水线使用。
	//		之后，当我们将它绑定到流水线的某个阶段时，就需要将它转换为对应的状态，比如 D3D12_RESOURCE_STATE_DEPTH_WRITE。
	//		之后，当我们将它从流水线的某个阶段解绑时，就需要将它转换回 D3D12_RESOURCE_STATE_COMMON。
	// 
	// 一个更完善的资源状态流程：// 如果是一个 RT 资源，则：
	//		将其设置为RT并绑定到流水线之前，应将其设置为 D3D12_RESOURCE_STATE_RENDER_TARGET；
	//		在 Present 执行之前，应将其转换为 D3D12_RESOURCE_STATE_PRESENT。
	//		如果这个 RT 指向的Buffer，需要在下一个 pass 用作SRV输入，则：
	//			如果该 pass 是 PixelShader，应将其转换为 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE。
	//			如果该 pass 是 ComputeShader，应将其转换为 D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE。
	// 
	//	总而言之，在一个渲染帧的整个过程中，需要根据各种情况，在Buffer使用前，将其转换成不同的资源状态。
	CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0f, 0x00);
	g_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &depthDesc, D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&m_pDepthStencilBuffer));
	m_pDepthStencilBuffer->SetName(L"Depth/Stencil Buffer");

	// 创建DSV描述符。和前面创建RTV描述符同理。
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_pDSVHeap->GetCPUDescriptorHandleForHeapStart());
	g_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, dsvHandle);
}

void D3D::CreateMyTexture()
{
	HRESULT hr;
	TexMetadata metadata;
	std::unique_ptr<ScratchImage> pImage = std::make_unique<ScratchImage>();

	hr = LoadFromWICFile(L"D:\\NixAssets\\hex-stones1\\albedo.png", WIC_FLAGS_NONE, &metadata, *pImage);
	//hr = LoadFromDDSFile(L"D:\\NixAssets\\dx12\\out\\1.dds", DDS_FLAGS_NONE, &metadata, *pImage);

	// Create the texture.
	{
		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC desc = {};
		desc.Width = static_cast<UINT>(metadata.width);
		desc.Height = static_cast<UINT>(metadata.height);
		desc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
		desc.DepthOrArraySize = (metadata.dimension == TEX_DIMENSION_TEXTURE3D)
			? static_cast<UINT16>(metadata.depth)
			: static_cast<UINT16>(metadata.arraySize);
		desc.Format = metadata.format;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		desc.SampleDesc.Count = 1;
		desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);

		CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_HEAP_PROPERTIES uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		g_pDevice->CreateCommittedResource(
			&defaultHeap,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_pTexture));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_pTexture.Get(), 0, desc.DepthOrArraySize * desc.MipLevels);
		CD3DX12_RESOURCE_DESC uploadBuffer = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		// Create the GPU upload buffer.
		g_pDevice->CreateCommittedResource(
			&uploadHeap,
			D3D12_HEAP_FLAG_NONE,
			&uploadBuffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pTextureUpload));
		m_pTextureUpload->SetName(L"textureUploadHeap temp");

		auto texDesc = m_pTexture->GetDesc();
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = new D3D12_PLACED_SUBRESOURCE_FOOTPRINT[texDesc.MipLevels];
		UINT* numRow = new UINT[texDesc.MipLevels];
		UINT64* numRowSizeInBytes = new UINT64[texDesc.MipLevels];
		size_t totalBytes;
		g_pDevice->GetCopyableFootprints(&texDesc, 0, texDesc.MipLevels, 0, layouts, numRow, numRowSizeInBytes, &totalBytes);

		void* mappedData;
		m_pTextureUpload->Map(0, nullptr, &mappedData);

		for (UINT mip = 0; mip < texDesc.MipLevels; mip++)
		{
			const Image* pImg = pImage->GetImage(mip, 0, 0);
			const BYTE* pSrcData = pImg->pixels; 
			BYTE* pDstData = reinterpret_cast<BYTE*>(mappedData) + layouts[mip].Offset;

			for (UINT y = 0; y < numRow[mip]; y++)
			{
				memcpy(pDstData + layouts[mip].Footprint.RowPitch * y, pSrcData + pImg->rowPitch * y, numRowSizeInBytes[mip]);
			}
		}

		m_pTextureUpload->Unmap(0, nullptr);

		for (UINT mip = 0; mip < texDesc.MipLevels; ++mip) {
			CD3DX12_TEXTURE_COPY_LOCATION dst(m_pTexture.Get(), mip); // 目标纹理的mip级别
			CD3DX12_TEXTURE_COPY_LOCATION src(m_pTextureUpload.Get(), layouts[mip]); // 上传堆的mip级别

			// 指定目标mip级别的x, y, z偏移量，通常为0
			g_pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		}

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		g_pCommandList->ResourceBarrier(1, &barrier);
	}

	m_pTexture->SetName(L"My Texture");

	pImage.reset();
}

void D3D::CreateCubeMap()
{
	HRESULT hr;
	TexMetadata metadata;
	std::unique_ptr<ScratchImage> pImage = std::make_unique<ScratchImage>();

	hr = LoadFromDDSFile(L"D:\\NixAssets\\HDR\\ballroom_128_Mip.dds", DDS_FLAGS_NONE, &metadata, *pImage);

	// Create the texture.
	{
		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC desc = {};
		desc.Width = static_cast<UINT>(metadata.width);
		desc.Height = static_cast<UINT>(metadata.height);
		desc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
		desc.DepthOrArraySize = (metadata.dimension == TEX_DIMENSION_TEXTURE3D)
			? static_cast<UINT16>(metadata.depth)
			: static_cast<UINT16>(metadata.arraySize);
		desc.Format = metadata.format;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		desc.SampleDesc.Count = 1;
		desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);

		CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_HEAP_PROPERTIES uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		g_pDevice->CreateCommittedResource(
			&defaultHeap,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_pCubeMap));

		UINT layoutSize = desc.DepthOrArraySize * desc.MipLevels;

		auto texDesc = m_pCubeMap->GetDesc();
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = new D3D12_PLACED_SUBRESOURCE_FOOTPRINT[layoutSize];
		UINT* numRow = new UINT[layoutSize];
		UINT64* numRowSizeInBytes = new UINT64[layoutSize];
		size_t totalBytes;
		g_pDevice->GetCopyableFootprints(&texDesc, 0, layoutSize, 0, layouts, numRow, numRowSizeInBytes, &totalBytes);
		
		CD3DX12_RESOURCE_DESC uploadBuffer = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);
		// Create the GPU upload buffer.
		g_pDevice->CreateCommittedResource(
			&uploadHeap,
			D3D12_HEAP_FLAG_NONE,
			&uploadBuffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pCubeMapUpload));
		m_pCubeMapUpload->SetName(L"textureUploadHeap temp");

		void* mappedData;
		m_pCubeMapUpload->Map(0, nullptr, &mappedData);

		for (UINT face = 0; face < texDesc.DepthOrArraySize; face++)
		{
			for (UINT mip = 0; mip < texDesc.MipLevels; mip++)
			{
				const Image* pImg = pImage->GetImage(mip, face, 0);
				const BYTE* pSrcData = pImg->pixels;
				BYTE* pDstData = reinterpret_cast<BYTE*>(mappedData) + layouts[mip].Offset;

				for (UINT y = 0; y < numRow[mip]; y++)
				{
					memcpy(pDstData + layouts[mip].Footprint.RowPitch * y, pSrcData + pImg->rowPitch * y, numRowSizeInBytes[mip]);
				}
			}
		}

		m_pCubeMapUpload->Unmap(0, nullptr);

		for (UINT l = 0; l < layoutSize; l++)
		{
			CD3DX12_TEXTURE_COPY_LOCATION dst(m_pCubeMap.Get(), l); // 目标纹理的mip级别
			CD3DX12_TEXTURE_COPY_LOCATION src(m_pCubeMapUpload.Get(), layouts[l]); // 上传堆的mip级别

			// 指定目标mip级别的x, y, z偏移量，通常为0
			g_pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		}

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pCubeMap.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		g_pCommandList->ResourceBarrier(1, &barrier);
	}

	m_pCubeMap->SetName(L"My Cube");

	pImage.reset();
}

void D3D::CreateGlobalConstantBuffers()
{
	// 创建全局常量缓冲区，存储MVP矩阵
	// 对这种每帧都需要更新的，准备一个上传堆就够了，不需要准备默认堆。默认堆是给静态的玩意（比如Mesh的纹理，VBIB）使用的
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(D3DUtil::CalcBufferViewSize(sizeof(MeshTransformData)) * 2);
	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc, // 资源描述
		D3D12_RESOURCE_STATE_GENERIC_READ, // 初始的资源状态为READ（允许CPU写入数据）
		nullptr,
		IID_PPV_ARGS(&m_pObjectCBUpload)
	);
	m_pObjectCBUpload->SetName(L"Object CB Upload");


	// 创建全局描述符堆
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // 常量缓冲区描述符类型
	cbvHeapDesc.NumDescriptors = 4; // 目前的规划是；1：Mesh的纹理的SRV；2: CubeMap的纹理的SRV；3-n：Mesh 1-(n-2)的全局MVP矩阵的View。
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // Shader 可见
	cbvHeapDesc.NodeMask = 0; // 0表示所有节点。节点就是GPU，多GPU的设备可以用这个参数来指定使用哪个GPU。
	g_pDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeapObject));

	// 描述符堆的堆头指针
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(m_pDescriptorHeapObject->GetCPUDescriptorHandleForHeapStart());

	// 创建纹理使用的SRV描述符试图
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // 默认的映射
	srvDesc.Format = m_pTexture->GetDesc().Format; // 纹理的格式
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = m_pTexture->GetDesc().MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0;
	srvDesc.Texture2D.PlaneSlice = 0;

	// 创建 SRV，并放在 描述符堆 的第1位
	g_pDevice->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, cbvHandle);

	// 创建CubeMap使用的SRV描述符试图
	D3D12_SHADER_RESOURCE_VIEW_DESC cubeSrvDesc = {};
	cubeSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // 默认的映射
	cubeSrvDesc.Format = m_pCubeMap->GetDesc().Format; // 纹理的格式
	cubeSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	cubeSrvDesc.TextureCube.MipLevels = m_pCubeMap->GetDesc().MipLevels;
	cubeSrvDesc.TextureCube.MostDetailedMip = 0;
	cubeSrvDesc.TextureCube.ResourceMinLODClamp = 0.0;

	// 创建 SRV，并放在 描述符堆 的第2位
	g_pDevice->CreateShaderResourceView(m_pCubeMap.Get(), &cubeSrvDesc, cbvHandle.Offset(1, m_nCBSRUAVDescriptorSize));

	{
		// 创建CBV，并放在 描述符堆 的第3、4位
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_pObjectCBUpload->GetGPUVirtualAddress(); // 常量缓冲区的GPU虚拟地址
		cbvDesc.SizeInBytes = D3DUtil::CalcBufferViewSize(sizeof(MeshTransformData)); // 常量缓冲区的大小
		g_pDevice->CreateConstantBufferView(&cbvDesc, cbvHandle.Offset(1, m_nCBSRUAVDescriptorSize));

		cbvDesc.BufferLocation += cbvDesc.SizeInBytes; // 偏移一位
		g_pDevice->CreateConstantBufferView(&cbvDesc, cbvHandle.Offset(1, m_nCBSRUAVDescriptorSize));
	}
}

void D3D::CreateRootSignature()
{
	// 创建静态采样器
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.ShaderRegister = 0; // HLSL中的寄存器号 = s0
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // 只在像素着色器中可见
	samplerDesc.RegisterSpace = 0;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 超出纹理坐标的部分，采用环绕方式
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MipLODBias = 0; // MipLOD偏移
	samplerDesc.MinLOD = 0.0f; // MipLOD的下界
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX; // MipLOD的上界
	samplerDesc.MaxAnisotropy = 1; // 各向异性过滤
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // 比较函数
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

	std::vector<D3D12_STATIC_SAMPLER_DESC> pSamplers = { samplerDesc };

	CD3DX12_ROOT_PARAMETER rootParam[2];
	{
		CD3DX12_DESCRIPTOR_RANGE range[1];
		range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, 0); // 1个SRV，slot 0~1，space0，从堆的第0位开始读取
		rootParam[0].InitAsDescriptorTable(_countof(range), range);
	}
	{
		CD3DX12_DESCRIPTOR_RANGE range[1];
		range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0); // 1个CBV，slot 0，space0，从堆的第2位开始读取
		rootParam[1].InitAsDescriptorTable(_countof(range), range);
	}
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(rootParam), rootParam, pSamplers.size(), pSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature;
	HRESULT hr;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	hr = g_pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
}

void D3D::CreateShaderAndPSO()
{
	ComPtr<ID3DBlob> pVSBlob;
	ComPtr<ID3DBlob> pErrorBlob;
	ComPtr<ID3DBlob> pPSBlob;

	HRESULT hr;

	hr = D3DCompileFromFile(L".\\Color.fx", nullptr, nullptr, "VSMain", "vs_5_0", D3DCOMPILE_DEBUG, 0, &pVSBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			std::string str = reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer());
			OutputDebugStringA(str.c_str());
		}
	}

	hr = D3DCompileFromFile(L".\\Color.fx", nullptr, nullptr, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG, 0, &pPSBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			std::string str = reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer());
			OutputDebugStringA(str.c_str());
		}
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {};
	defaultRenderTargetBlendDesc.BlendEnable = false;
	defaultRenderTargetBlendDesc.LogicOpEnable = false;
	defaultRenderTargetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	defaultRenderTargetBlendDesc.DestBlend = D3D12_BLEND_ONE;
	defaultRenderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	defaultRenderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	defaultRenderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
	defaultRenderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	defaultRenderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;

	D3D12_BLEND_DESC blendState = {};
	blendState.AlphaToCoverageEnable = false;
	blendState.IndependentBlendEnable = false;
	defaultRenderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		blendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

	D3D12_RASTERIZER_DESC rasterizerState = {};
	rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerState.FrontCounterClockwise = false;
	rasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.MultisampleEnable = false;
	rasterizerState.AntialiasedLineEnable = false;
	rasterizerState.ForcedSampleCount = 0;
	rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_DEPTH_STENCIL_DESC depthStencilState = {};
	depthStencilState.DepthEnable = true;
	depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilState.StencilEnable = false;
	depthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
	{
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_COMPARISON_FUNC_ALWAYS
	};
	depthStencilState.FrontFace = defaultStencilOp;
	depthStencilState.BackFace = defaultStencilOp;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_pRootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize());
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.BlendState = blendState;
	psoDesc.RasterizerState = rasterizerState;
	psoDesc.DepthStencilState = depthStencilState;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.SampleMask = UINT32_MAX;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	g_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D::GetSwapChainBackBufferRTV()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart(), m_nRTVDescriptorSize, m_pSwapChain->GetCurrentBackBufferIndex());
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D::GetSwapChainBackBufferDSV()
{
	return m_pDSVHeap->GetCPUDescriptorHandleForHeapStart();
}

ID3D12Resource* D3D::GetSwapChainBackBuffer() const
{
	return m_pSwapChainRT[m_pSwapChain->GetCurrentBackBufferIndex()].Get();
}

void D3D::Draw()
{
	HRESULT hr;
	hr = g_pCommandAllocator->Reset();

	hr = g_pCommandList->Reset(g_pCommandAllocator.Get(), nullptr);

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
	g_pCommandList->ClearRenderTargetView(currSwapChainRTV, m_pSwapChain->GetCurrentBackBufferIndex() ? DirectX::Colors::LightSteelBlue : DirectX::Colors::LightSteelBlue, 0, nullptr);
	g_pCommandList->ClearDepthStencilView(currSwapChainDSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	g_pCommandList->OMSetRenderTargets(1, &currSwapChainRTV, true, &currSwapChainDSV);

	RenderMeshes();

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

	if (m_pFence->GetCompletedValue() < m_currFenceIdx)
	{
		// 创建一个 Windows 事件 fenceEvent。
		HANDLE fenceEvent = CreateEvent(nullptr, false, false, nullptr);

		// 通过下面的方法告知 GPU 值达到 m_currFenceIdx 事件时，向 CPU 推送一个 fenceEvent。
		m_pFence->SetEventOnCompletion(m_currFenceIdx, fenceEvent);

		// 让 Windows 持续等待这个 fence。
		// 换言之，CPU 这边将持续等待，直到 GPU 那边的值确实的变化到 m_currFenceIdx。
		WaitForSingleObject(fenceEvent, INFINITE);

		// 当执行到这里，说明 GPU 的值已经确实的变化到 m_currFenceIdx 了。
		// 这时就可以将这个事件关闭掉了。
		// 这样也就完成了一次 CPU 和 GPU 之间的通信同步。
		CloseHandle(fenceEvent);
	}
}

void D3D::RenderMeshes()
{
	g_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_pDescriptorHeapObject.Get() };
	g_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	g_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pCommandList->SetPipelineState(m_pPipelineState.Get());

	g_pCommandList->SetGraphicsRootDescriptorTable(0, m_pDescriptorHeapObject->GetGPUDescriptorHandleForHeapStart());

	// World 的部分需要逐 Mesh 更新
	void* pTransformData;
	m_pObjectCBUpload->Map(0, nullptr, (void**)&pTransformData);

	m_pMesh->Update();
	memcpy(pTransformData, &g_cbObjectData, sizeof(MeshTransformData));

	m_pMeshCube->Update();
	memcpy(reinterpret_cast<char*>(pTransformData) + D3DUtil::CalcBufferViewSize(sizeof(MeshTransformData)), &g_cbObjectData, sizeof(MeshTransformData));

	m_pObjectCBUpload->Unmap(0, nullptr);

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_pDescriptorHeapObject->GetGPUDescriptorHandleForHeapStart(), 0, m_nCBSRUAVDescriptorSize);
	g_pCommandList->SetGraphicsRootDescriptorTable(1, gpuHandle.Offset(2, m_nCBSRUAVDescriptorSize));
	m_pMesh->Render();

	g_pCommandList->SetGraphicsRootDescriptorTable(1, gpuHandle.Offset(1, m_nCBSRUAVDescriptorSize));
	m_pMeshCube->Render();
}

void D3D::Release()
{
	if (m_pMesh)
	{
		delete m_pMesh;
		m_pMesh = nullptr;
	}
}

UINT D3DUtil::CalcBufferViewSize(UINT sizeInBytes)
{
	return (sizeInBytes + 255) & ~255;
}
