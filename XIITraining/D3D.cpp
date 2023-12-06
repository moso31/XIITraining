#include "D3D.h"
#include "Mesh.h"

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

	CreateRootSignature();

	// ������������
	CreateDescriptorHeap();

	CreateGlobalConstantBuffers();

	CreateShaderAndPSO();

	m_pMesh = new Mesh();
	m_pMesh->InitBox();
	
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

void D3D::CreateMyTexture()
{
	HRESULT hr;
	TexMetadata metadata;
	std::unique_ptr<ScratchImage> pImage = std::make_unique<ScratchImage>();

	//hr = LoadFromWICFile(L"D:\\NixAssets\\hex-stones1\\albedo.png", WIC_FLAGS_NONE, &metadata, *pImage);
	hr = LoadFromTGAFile(L"D:\\1.tga", TGA_FLAGS_DEFAULT_SRGB, &metadata, *pImage);
	//hr = LoadFromDDSFile(L"D:\\NixAssets\\checkboard.dds", DDS_FLAGS_NONE, &metadata, *pImage);

	hr = CreateTextureEx(g_pDevice.Get(), metadata, D3D12_RESOURCE_FLAG_NONE, CREATETEX_DEFAULT, &m_pTexture);
	m_pTexture->SetName(L"My Texture");

	pImage.reset();
}

void D3D::CreateGlobalConstantBuffers()
{
	// ����ȫ�ֳ������������洢MVP����
	// ������ÿ֡����Ҫ���µģ�׼��һ���ϴ��Ѿ͹��ˣ�����Ҫ׼��Ĭ�϶ѡ�Ĭ�϶��Ǹ���̬�����⣨����Mesh������VBIB��ʹ�õ�
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(D3DUtil::CalcBufferViewSize(sizeof(MeshTransformData)));
	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc, // ��Դ����
		D3D12_RESOURCE_STATE_GENERIC_READ, // ��ʼ����Դ״̬ΪREAD������CPUд�����ݣ�
		nullptr,
		IID_PPV_ARGS(&m_pObjectCBUpload)
	);
	m_pObjectCBUpload->SetName(L"Object CB Upload");

	// ����ȫ����������
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // ��������������������
	cbvHeapDesc.NumDescriptors = 2; // Ŀǰ�Ĺ滮��2����������1��ȫ��MVP����2��Mesh�������SRV��
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // Shader �ɼ�
	cbvHeapDesc.NodeMask = 0; // 0��ʾ���нڵ㡣�ڵ����GPU����GPU���豸���������������ָ��ʹ���ĸ�GPU��
	g_pDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeapObject));

	// �������ѵĶ�ͷָ��
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(m_pDescriptorHeapObject->GetCPUDescriptorHandleForHeapStart());

	// ����ȫ�ֳ�������������������ͼ
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_pObjectCBUpload->GetGPUVirtualAddress(); // ������������GPU�����ַ
	cbvDesc.SizeInBytes = D3DUtil::CalcBufferViewSize(sizeof(MeshTransformData)); // �����������Ĵ�С

	// ����CBV�������� �������� �ĵ�1λ
	g_pDevice->CreateConstantBufferView(&cbvDesc, cbvHandle);

	// ����ʵ��Ӧ�õ�Mesh�ϵ�������Դ
	CreateMyTexture();

	// ��������ʹ�õ�SRV��������ͼ
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // Ĭ�ϵ�ӳ��
	srvDesc.Format = m_pTexture->GetDesc().Format; // ����ĸ�ʽ
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	// ���� SRV�������� �������� �ĵ�2λ
	g_pDevice->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, cbvHandle.Offset(1, m_nCBSRUAVDescriptorSize));
}

void D3D::CreateRootSignature()
{
	// ������̬������
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.ShaderRegister = 0; // HLSL�еļĴ����� = s0
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // ֻ��������ɫ���пɼ�
	samplerDesc.RegisterSpace = 0;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // ������������Ĳ��֣����û��Ʒ�ʽ
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MipLODBias = 0; // MipLODƫ��
	samplerDesc.MinLOD = 0.0f; // MipLOD���½�
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX; // MipLOD���Ͻ�
	samplerDesc.MaxAnisotropy = 1; // �������Թ���
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // �ȽϺ���
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

	std::vector<D3D12_STATIC_SAMPLER_DESC> pSamplers = { samplerDesc };

	// ������ǩ���������������ȾMesh�ĸ�ǩ���Ľṹ���£�
	// -��ǩ��0
	// --������0����������
	// ---������0��CBV
	// --������1����������
	// ---������0��SRV
	// ÿ�� Render() ʱ�����ø�ǩ�����ɡ�
	CD3DX12_ROOT_PARAMETER rootParam[2];
	CD3DX12_DESCRIPTOR_RANGE range[2];
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, -1); // 1��CBV����0�ſ�ʼ��ʹ�� space0���Ӷѵĵ�1λ��ʼ��ȡ
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, -1); // 1��SRV����0�ſ�ʼ��ʹ�� space0���Ӷѵĵ�2λ��ʼ��ȡ
	rootParam[0].InitAsDescriptorTable(1, &range[0]); 
	rootParam[1].InitAsDescriptorTable(1, &range[1]); 
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(rootParam), rootParam, 1, pSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature;
	D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	HRESULT hr;
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
	depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
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

void D3D::Update()
{
	// ��ʱ��ʹ�ù̶����������
	g_cbObjectData.m_view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, -4.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)).Transpose();
	g_cbObjectData.m_proj = Matrix::CreatePerspectiveFieldOfView(60.0f / 180.0f * 3.1415926f, (float)m_width / (float)m_height, 0.01f, 100.0f).Transpose();

	// World �Ĳ�����Ҫ�� Mesh ����
	m_pMesh->Update();

	// ����ȫ�ֳ��������������� DX11 �е� UpdateSubresource
	MeshTransformData* pTransformData;
	m_pObjectCBUpload->Map(0, nullptr, (void**)&pTransformData);
	*pTransformData = g_cbObjectData;
	m_pObjectCBUpload->Unmap(0, nullptr);
}

void D3D::Render()
{
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
	g_pCommandList->ClearRenderTargetView(currSwapChainRTV, m_pSwapChain->GetCurrentBackBufferIndex() ?  DirectX::Colors::LightSteelBlue : DirectX::Colors::LightSteelBlue, 0, nullptr);
	g_pCommandList->ClearDepthStencilView(currSwapChainDSV, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	g_pCommandList->OMSetRenderTargets(1, &currSwapChainRTV, true, &currSwapChainDSV);

	RenderMeshes();

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

void D3D::RenderMeshes()
{
	// CBV�� �󶨵� cmdList
	g_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_pDescriptorHeapObject.Get() };
	g_pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	g_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pCommandList->SetPipelineState(m_pPipelineState.Get());

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_pDescriptorHeapObject->GetGPUDescriptorHandleForHeapStart());
	g_pCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);
	g_pCommandList->SetGraphicsRootDescriptorTable(1, gpuHandle.Offset(1, m_nCBSRUAVDescriptorSize));

	m_pMesh->Render();
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
