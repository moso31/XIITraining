#include "D3D.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "DescriptorAllocator.h"
#include "CBufferAllocator.h"

void D3D::Init()
{
	// ���� D3D12 ���Բ�
#if defined(DEBUG) || defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	debugController->EnableDebugLayer();
#endif

	HRESULT hr;

	// ���� DXGI ����
	hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_pDXGIFactory));

	// �� DXGI ������ ö�ٵ�һ����������Ĭ�ϵ��Կ���
	ComPtr<IDXGIAdapter1> pAdapter;
	m_pDXGIFactory->EnumAdapters1(0, &pAdapter);

	// ת���� IDXGIAdapter4��
	// IDXGIAdapter �����ʼ�汾�����Ƽ�ʹ�ã�
	// IDXGIAdapter1����������Կ��Ĺ�Ӧ��ID���豸ID����ϵͳID�ȡ�
	// IDXGIAdapter2��������Կ����й��Ĺ���
	// IDXGIAdapter3����һ�������˶��Դ�ʹ������Ĳ�ѯ���ܡ�
	// IDXGIAdapter4 �ṩ�˶� GPU ���߼���Ŀ��ƺͼ��ӡ��ر��Ƕ�Ӳ���������ݵķ��ʡ�
	ComPtr<IDXGIAdapter4> pAdapter4;
	hr = pAdapter.As(&pAdapter4);

	// ���� D3D12 �豸
	// ID3D12Device Ҳ�ֳ����ɰ汾�����ڰ汾̫���ˣ�������Ҫ��˵��
	// �� ID3D12Device4 ��ʼ�������˶Թ���׷�ٹ��ܵĽӿ�֧�֡�
	// Խ�ϵİ汾�����Ժ��ȶ���Խǿ���Լ�����ѡ��
	hr = D3D12CreateDevice(pAdapter4.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&g_pDevice));

	// ���� Fence ����ͬ�� CPU �� GPU �Ĳ�����
	hr = g_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));

	// ��ȡ �������� �� ���������С��
	// ���������С���� DX12 ���¸���� DX12 �У�ʹ�� �������� �洢 ��������
	// Ŀǰ DX12 �����°汾���ֳ����ĸ����͵Ķѣ�RTV��DSV��Sampler��CBV/SRV/UAV��
	// �ٸ����ӣ�����������һ�� DSV�������ѣ�����ȫ�� DSV��������
	// ����������Ҫ������еĵ�3��������������������Ҫ��������ķ�������ȡ DSVDescriptorSize������ = 32
	// ��ô���������ڶ��е���ʼ�ֽھ��� 32 * 3 = 96��
	// note��DX12�У�һ�����������µ����� ������ ����һ�¡�������� DSV/RTV/XX... ����һ������������
	// ��Ϊ��ѧ�ߣ������Ȳ������ǵĺ�����ʲô���Ȱѻ�ȡ��С���ֽ�ƫ�Ƶĸ������ͺá�
	m_nRTVDescriptorSize = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_nDSVDescriptorSize = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_nSamplerDescriptorSize = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	m_nCBSRUAVDescriptorSize = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// ����������С�����������������б�
	CreateCommandObjects();

	// ����������
	CreateSwapChain();

	// ����������������
	g_pDescriptorAllocator = new DescriptorAllocator(g_pDevice.Get());
	
	// ���� CBuffer ������
	g_pCBufferAllocator = new CBufferAllocator(g_pDevice.Get(), g_pDescriptorAllocator);

	// ������������
	CreateDescriptorHeap();

	// ����ʵ��Ӧ�õ�Mesh�ϵ�������Դ
	m_pTextureBox = new Texture();
	m_pTextureBox->Load("D:\\NixAssets\\rustediron2\\albedo.png", "My Texture");
	m_pTextureBox->AddSRV(TextureType_2D);

	m_pTextureCubeMap = new Texture();
	m_pTextureCubeMap->Load("D:\\NixAssets\\HDR\\ballroom_4k.dds", "My Cube");
	m_pTextureCubeMap->AddSRV(TextureType_Cube);

	// �������ʣ�ָ�������ֶ����롣
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

	CreateCBufferPerFrame();

	// ��ģ��
	m_pMesh = new Mesh();
	m_pMesh->InitBox();
	m_pMesh->SetScale(1.0f, 1.0f, 1.0f);
	m_pMesh->SetRotate(true);

	m_pMeshCube = new Mesh();
	m_pMeshCube->InitBox();
	m_pMeshCube->SetScale(100.0f, 100.0f, 100.0f);
	//m_pMeshCube->SetScale(0.1f, 0.1f, 0.1f);
	m_pMeshCube->SetRotate(true);

	// ��ģ�Ͱ����
	m_pMesh->SetMaterial(pMaterialBox);
	m_pMeshCube->SetMaterial(pMaterialCubeMap);
	
	// ��ʼ�����̽�����Ĭ�Ϲر������б�
	// ��Ȼ���ǹٷ��涨��������һ������ʵ��������
	// ��ʼ�ر������б���Ҫ������б��ʱ��ʹ�� Reset ���´�����Ȼ���ټ�¼�µ����
	// �������Ա��� CommandList �ں��������б������߼��������������Ⱦ����
	// Ҫ����Close�����������������б�֮��ĵ����߼��ǳ���ȷ�����ǣ��α��أ�
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

	// �����������
	hr = g_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_pCommandQueue));

	// �������������
	hr = g_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_pCommandAllocator));

	// ���������б�������� ��������� ����
	// ͬʱ�趨��ʼ��Ⱦ����״̬ = nullptr��Ĭ��״̬��
	hr = g_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&g_pCommandList));
}

void D3D::CreateSwapChain()
{
	HRESULT hr;

	// ��ȡ���ڴ�С
	RECT rect;
	GetClientRect(g_hWnd, &rect);
	UINT width = rect.right - rect.left;
	UINT height = rect.bottom - rect.top;

	// ��� SwapChain �����ṹ��
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 8 λ RGBA
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60; // ˢ����
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.SampleDesc.Count = 1; // MSAA4x ����
	swapChainDesc.SampleDesc.Quality = 0; 
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // ����RT
	swapChainDesc.BufferCount = m_swapChainBufferCount; // ˫����
	swapChainDesc.OutputWindow = g_hWnd; // ���ھ��
	swapChainDesc.Windowed = true; // ����ģʽ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // ��תģʽ
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ����ȫ���л�

	// ���������� ���� ת���� IDXGISwapChain4
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
	RTVHeapDesc.NumDescriptors = m_swapChainBufferCount; // RTV�������� ������������������Ӧ�� SwapChain ��˫���塣

	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVHeapDesc.NodeMask = 0;
	DSVHeapDesc.NumDescriptors = 1;

	g_pDevice->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_pRTVHeap));
	g_pDevice->CreateDescriptorHeap(&DSVHeapDesc, IID_PPV_ARGS(&m_pDSVHeap));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pRTVHeap->GetCPUDescriptorHandleForHeapStart());

	// ����������������RT
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		// ��ȡ����һ��RT
		m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pSwapChainRT[i]));
		std::wstring wstr = std::wstring(L"SwapChainRT") + std::to_wstring(i);
		m_pSwapChainRT[i]->SetName(wstr.c_str());

		// Ϊ�䴴��һ��RTV��������
		// �����ϤDX11�Ļ�����ʵView��Sampler��������������
		// ���ڶ�Ӧ��RTV�������Ѵ��������RTV��������ʱ��DX12 Device����û�� "����������" ������˵����
		// ���Ǳ�����DX11�ķ�� ���� "���� View"/"����Sampler"��
		// ���ǣ����DX11������������
		//		DX11��ֱ�Ӵ���RTV����
		//		DX12����RTV�������������һ��ָ��RTV��ָ�롣
		g_pDevice->CreateRenderTargetView(m_pSwapChainRT[i].Get(), nullptr, rtvHandle); // �� rtvHandle ��Ӧ�� RTV�������������һ��RTV��
		rtvHandle.Offset(1, m_nRTVDescriptorSize);
	}

	// �������Buffer��Դ��desc
	// ��������BackBuffer����Ҫ�����������ǰ��ķ���������������������BackBufferʱ��ʹ��ר�ŵ�DXGI_SWAP_CHAIN_DESC
	//		������������봴��һ����ͨRT������Ҫʹ������� D3D12_RESOURCE_DESC ������
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

	// ��DX12�У�����������Դ��ʱ����Ҫ��ȷ�������Դ���ύ��ʲô���Ķ��
	// ������ν�Ķѣ�����GPU�е������Դ�顣���ִ�GPU�У��Դ������Ͱ�����
	// 1. Ĭ�϶ѣ�GPU �ɷ��ʣ�CPU ��Զ���ɷ��ʡ�
	// 2. �ϴ��ѣ�������е���Դʼ���ڵȴ������Ǵ� CPU �ϴ��� GPU��
	// 3. �ض��ѣ�������е���Դʼ���ڵȴ� CPU ��ȡ���ǡ�
	// 4. �Զ���ѣ���������Ը߼���������ʱ���ÿ���
	D3D12_HEAP_PROPERTIES heapProps;
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;  // ָ��������Ϊ 1.Ĭ�϶�
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;  // CPUҳ�����Բ�������Ĭ�϶�
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;  // �ڴ��ƫ������Ϊδ֪����Ϊ����Ĭ�϶�
	heapProps.CreationNodeMask = 0;  // �����ڵ����룬���ڶ�GPUϵͳ��0��ʾ����GPU�ڵ�
	heapProps.VisibleNodeMask = 0;   // �ɼ��ڵ����룬���ڶ�GPUϵͳ��0��ʾ����GPU�ڵ�


	// �������Buffer��Դ����
	// ��Ȼ����������������������RTҲ����Ҫ���������
	// ����Ĳ����У�û���ᵽ����������
	// D3D12_HEAP_FLAG_NONE���Ȳ��ùܣ���������none���С�
	// 
	// D3D12_RESOURCE_STATE_COMMON����Դ״̬��COMMON��ʾ������Դת�������еĳ�ʼ״̬��
	// ��Դ״̬���������� DX12 ���¸�������ܱ����� DX11 Ҳ����Դת����������װ�������㣬DX11 API������ͨ������Ҫ������Щ����
	//		������� ���buffer Ϊ�����ʼ��״̬�� D3D12_RESOURCE_STATE_COMMON����ʾ�����Դ�ǿ��õģ����ǻ�û�б��κ���ˮ��ʹ�á�
	//		֮�󣬵����ǽ����󶨵���ˮ�ߵ�ĳ���׶�ʱ������Ҫ����ת��Ϊ��Ӧ��״̬������ D3D12_RESOURCE_STATE_DEPTH_WRITE��
	//		֮�󣬵����ǽ�������ˮ�ߵ�ĳ���׶ν��ʱ������Ҫ����ת���� D3D12_RESOURCE_STATE_COMMON��
	// 
	// һ�������Ƶ���Դ״̬���̣�// �����һ�� RT ��Դ����
	//		��������ΪRT���󶨵���ˮ��֮ǰ��Ӧ��������Ϊ D3D12_RESOURCE_STATE_RENDER_TARGET��
	//		�� Present ִ��֮ǰ��Ӧ����ת��Ϊ D3D12_RESOURCE_STATE_PRESENT��
	//		������ RT ָ���Buffer����Ҫ����һ�� pass ����SRV���룬��
	//			����� pass �� PixelShader��Ӧ����ת��Ϊ D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE��
	//			����� pass �� ComputeShader��Ӧ����ת��Ϊ D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE��
	// 
	//	�ܶ���֮����һ����Ⱦ֡�����������У���Ҫ���ݸ����������Bufferʹ��ǰ������ת���ɲ�ͬ����Դ״̬��
	CD3DX12_CLEAR_VALUE clearValue(DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0f, 0x00);
	g_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &depthDesc, D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&m_pDepthStencilBuffer));
	m_pDepthStencilBuffer->SetName(L"Depth/Stencil Buffer");

	// ����DSV����������ǰ�洴��RTV������ͬ��
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_pDSVHeap->GetCPUDescriptorHandleForHeapStart());
	g_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, dsvHandle);
}

void D3D::CreateCBufferPerFrame()
{
	// ��ʱ��ʹ�ù̶����������
	g_cbPerFrame.m_view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, -4.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)).Transpose();
	g_cbPerFrame.m_proj = Matrix::CreatePerspectiveFieldOfView(60.0f / 180.0f * 3.1415926f, (float)m_width / (float)m_height, 0.01f, 300.0f).Transpose();

	g_pCBufferAllocator->AllocCBV(g_cbPerFrame, g_cbDataGPUVirtualAddr, g_cbDataCBufferPageIndex, g_cbDataByteOffset);
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

void D3D::Prepare()
{
	for (auto& pMaterial : m_pMaterials)
	{
		// ��ȡ�������ʹ�õ����� non-shader-visible (cpu) ������
		const size_t* pDescriptors = pMaterial->GetViewsGroup();
		const size_t pDescriptorsSize = pMaterial->GetViewsGroupSize();

		// ����Щ ������ ׷�ӵ� shader-visible (gpu) ���������� 
		UINT renderHeapOffset = g_pDescriptorAllocator->AppendToRenderHeap(pDescriptors, pDescriptorsSize);
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = g_pDescriptorAllocator->GetRenderHeap()->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += renderHeapOffset * g_pDescriptorAllocator->GetRenderHeapDescriptorByteSize();

		pMaterial->SetGPUHandle(gpuHandle);
	}
}

void D3D::Draw()
{
	Prepare();
	Update();

	HRESULT hr;
	hr = g_pCommandAllocator->Reset();

	hr = g_pCommandList->Reset(g_pCommandAllocator.Get(), nullptr);

	// �����ӿ�
	CD3DX12_VIEWPORT vp(0.0f, 0.0f, (float)m_width, (float)m_height);
	g_pCommandList->RSSetViewports(1, &vp);

	// ����һ�����ӿڴ�С��ͬ�ļ��о���
	CD3DX12_RECT scissorRect(0, 0, (LONG)m_width, (LONG)m_height);
	g_pCommandList->RSSetScissorRects(1, &scissorRect);

	// ���õ�ǰ֡ backBuffer ����Դ״̬Ϊ RENDERTARGET��
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(GetSwapChainBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	g_pCommandList->ResourceBarrier(1, &barrier);

	// ���õ�ǰ֡ depthBuffer ����Դ״̬Ϊ DEPTHWRITE��
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	g_pCommandList->ResourceBarrier(1, &barrier);

	auto currSwapChainRTV = GetSwapChainBackBufferRTV();
	auto currSwapChainDSV = GetSwapChainBackBufferDSV();
	g_pCommandList->ClearRenderTargetView(currSwapChainRTV, m_pSwapChain->GetCurrentBackBufferIndex() ? DirectX::Colors::LightSteelBlue : DirectX::Colors::LightSteelBlue, 0, nullptr);
	g_pCommandList->ClearDepthStencilView(currSwapChainDSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	g_pCommandList->OMSetRenderTargets(1, &currSwapChainRTV, true, &currSwapChainDSV);

	{
		g_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D12DescriptorHeap* pRenderHeap = g_pDescriptorAllocator->GetRenderHeap();
		ID3D12DescriptorHeap* ppHeaps[] = { pRenderHeap };
		g_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		for (auto& pMat : m_pMaterials)
		{
			pMat->Render();
		}
	}

	// Clear��SetRTִ���꣬����Դ״̬���û� PRESENT
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(GetSwapChainBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	g_pCommandList->ResourceBarrier(1, &barrier);

	// Clear��SetRTִ���꣬����Դ״̬���û� COMMON
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
	// ͨ�� Signal����֪GPU���� Queue ִ����Ϻ󣬽�ֵ���õ� m_currFenceIdx
	g_pCommandQueue->Signal(m_pFence.Get(), m_currFenceIdx);

	if (m_pFence->GetCompletedValue() < m_currFenceIdx)
	{
		// ����һ�� Windows �¼� fenceEvent��
		HANDLE fenceEvent = CreateEvent(nullptr, false, false, nullptr);

		// ͨ������ķ�����֪ GPU ֵ�ﵽ m_currFenceIdx �¼�ʱ���� CPU ����һ�� fenceEvent��
		m_pFence->SetEventOnCompletion(m_currFenceIdx, fenceEvent);

		// �� Windows �����ȴ���� fence��
		// ����֮��CPU ��߽������ȴ���ֱ�� GPU �Ǳߵ�ֵȷʵ�ı仯�� m_currFenceIdx��
		WaitForSingleObject(fenceEvent, INFINITE);

		// ��ִ�е����˵�� GPU ��ֵ�Ѿ�ȷʵ�ı仯�� m_currFenceIdx �ˡ�
		// ��ʱ�Ϳ��Խ�����¼��رյ��ˡ�
		// ����Ҳ�������һ�� CPU �� GPU ֮���ͨ��ͬ����
		CloseHandle(fenceEvent);
	}
}

void D3D::Update()
{
	g_pCBufferAllocator->UpdateCBData(g_cbPerFrame, g_cbDataCBufferPageIndex, g_cbDataByteOffset);

	for (auto& pMat : m_pMaterials)
	{
		for (auto& pMesh : pMat->GetSubMeshes())
		{
			pMesh->Update();
		}
	}
}

void D3D::RenderMeshes()
{
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
