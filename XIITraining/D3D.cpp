#include "D3D.h"

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
	hr = D3D12CreateDevice(pAdapter4.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pDevice));

	// ���� Fence ����ͬ�� CPU �� GPU �Ĳ�����
	hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));

	// ��ȡ �������� �� ���������С��
	// ���������С���� DX12 ���¸���� DX12 �У�ʹ�� �������� �洢 ��������
	// Ŀǰ DX12 �����°汾���ֳ����ĸ����͵Ķѣ�RTV��DSV��Sampler��CBV/SRV/UAV��
	// �ٸ����ӣ�����������һ�� DSV�������ѣ�����ȫ�� DSV��������
	// ����������Ҫ������еĵ�3��������������������Ҫ��������ķ�������ȡ DSVDescriptorSize������ = 32
	// ��ô���������ڶ��е���ʼ�ֽھ��� 32 * 3 = 96��
	// note��DX12�У�һ�����������µ����� ������ ����һ�¡�������� DSV/RTV/XX... ����һ������������
	// ��Ϊ��ѧ�ߣ������Ȳ������ǵĺ�����ʲô���Ȱѻ�ȡ��С���ֽ�ƫ�Ƶĸ������ͺá�
	int nRTVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	int nDSVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	int nSamplerDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	int nCBSRUAVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// ����������
	CreateSwapChain();

	// ����������С�����������������б�
	CreateCommandObjects();

	// ������������
	CreateDescriptorHeap();
}

void D3D::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; 
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT hr;

	// �����������
	hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));

	// �������������
	hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator));

	// ���������б�������� ��������� ����
	// ͬʱ�趨��ʼ��Ⱦ����״̬ = nullptr��Ĭ��״̬��
	hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList));

	// ��ʼĬ�Ϲر������б�
	// ��Ȼ���ǹٷ��涨��������һ������ʵ��������
	// ��ʼ�ر������б���Ҫ������б��ʱ��ʹ�� Reset ���´�����Ȼ���ټ�¼�µ����
	// �������Ա��� CommandList �ں��������б������߼��������������Ⱦ����
	// Ҫ����Close�����������������б�֮��ĵ����߼��ǳ���ȷ�����ǣ��α��أ�
	m_pCommandList->Close();
}

void D3D::CreateSwapChain()
{
	HRESULT hr;

	// ��ȡ���ڴ�С
	RECT rect;
	GetClientRect(m_hWnd, &rect);
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
	swapChainDesc.OutputWindow = m_hWnd; // ���ھ��
	swapChainDesc.Windowed = true; // ����ģʽ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // ��תģʽ
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ����ȫ���л�

	// ���������� ���� ת���� IDXGISwapChain4
	ComPtr<IDXGISwapChain> pSwapChain;
	hr = m_pDXGIFactory->CreateSwapChain(m_pCommandQueue.Get(), &swapChainDesc, &pSwapChain);
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
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	DSVHeapDesc.NodeMask = 0;
	DSVHeapDesc.NumDescriptors = m_swapChainBufferCount;

	m_pDevice->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_pRTVHeap));
	m_pDevice->CreateDescriptorHeap(&DSVHeapDesc, IID_PPV_ARGS(&m_pDSVHeap));

	// ����������������RT
	ComPtr<ID3D12Resource> pSwapChainRT[2]; // m_swapChainBufferCount == 2
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		// ��ȡ����һ��RT
		m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pSwapChainRT[i]));

		// Ϊ�䴴��һ��RTV
		m_pSwapChain->
	}
}

void D3D::Render()
{
	m_backBufferIndex++;


	// ��λʵ�ʵ�RTV // ÿ֡ʹ�ò�ͬ�� RTV��������Ҫƫ��ָ��
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pRTVHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr = { rtvHandle.ptr + (UINT64)(nRTVDescriptorSize * m_backBufferIndex) }; 

	// ��λʵ�ʵ�DSV
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = pDSVHeap->GetCPUDescriptorHandleForHeapStart();

}
