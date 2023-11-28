#include "D3D.h"

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
	hr = D3D12CreateDevice(pAdapter4.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pDevice));

	// 创建 Fence 用于同步 CPU 和 GPU 的操作。
	hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));

	// 获取 描述符堆 的 句柄增量大小。
	// 句柄增量大小：是 DX12 的新概念。在 DX12 中，使用 描述符堆 存储 描述符。
	// 目前 DX12 的最新版本，分成了四个类型的堆：RTV，DSV，Sampler，CBV/SRV/UAV。
	// 举个例子，假设我们有一个 DSV描述符堆，里面全是 DSV描述符。
	// 现在我们想要这个堆中的第3个描述符，于是首先需要调用下面的方法，获取 DSVDescriptorSize，例如 = 32
	// 那么该描述符在堆中的起始字节就是 32 * 3 = 96。
	// note：DX12中，一个描述符堆下的所有 描述符 类型一致。不会出现 DSV/RTV/XX... 混在一个堆里的情况。
	// 作为初学者，可以先不管它们的含义是什么。先把获取大小、字节偏移的概念理解就好。
	int nRTVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	int nDSVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	int nSamplerDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	int nCBSRUAVDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 创建交换链
	CreateSwapChain();

	// 创建命令队列、命令分配器、命令列表
	CreateCommandObjects();

	// 创建描述符堆
	CreateDescriptorHeap();
}

void D3D::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; 
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT hr;

	// 创建命令队列
	hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));

	// 创建命令分配器
	hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator));

	// 创建命令列表，并将其和 命令分配器 关联
	// 同时设定初始渲染管线状态 = nullptr（默认状态）
	hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList));

	// 初始默认关闭命令列表
	// 虽然并非官方规定，但这是一个常见实践做法。
	// 初始关闭命令列表，需要用这个列表的时候，使用 Reset 重新打开它，然后再记录新的命令。
	// 这样可以避免 CommandList 在后续操作中被其它逻辑掺入命令，导致渲染错误。
	// 要跳过Close，除非你对这个命令列表之后的调用逻辑非常明确。但是，何必呢？
	m_pCommandList->Close();
}

void D3D::CreateSwapChain()
{
	HRESULT hr;

	// 获取窗口大小
	RECT rect;
	GetClientRect(m_hWnd, &rect);
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
	swapChainDesc.OutputWindow = m_hWnd; // 窗口句柄
	swapChainDesc.Windowed = true; // 窗口模式
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 翻转模式
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // 允许全屏切换

	// 创建交换链 并且 转换成 IDXGISwapChain4
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
	RTVHeapDesc.NumDescriptors = m_swapChainBufferCount; // RTV描述符堆 内置两张描述符，对应了 SwapChain 的双缓冲。

	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	DSVHeapDesc.NodeMask = 0;
	DSVHeapDesc.NumDescriptors = m_swapChainBufferCount;

	m_pDevice->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_pRTVHeap));
	m_pDevice->CreateDescriptorHeap(&DSVHeapDesc, IID_PPV_ARGS(&m_pDSVHeap));

	// 创建交换链的两个RT
	ComPtr<ID3D12Resource> pSwapChainRT[2]; // m_swapChainBufferCount == 2
	for (int i = 0; i < m_swapChainBufferCount; i++)
	{
		// 获取其中一个RT
		m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pSwapChainRT[i]));

		// 为其创建一个RTV
		m_pSwapChain->
	}
}

void D3D::Render()
{
	m_backBufferIndex++;


	// 定位实际的RTV // 每帧使用不同的 RTV，所以需要偏移指针
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pRTVHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr = { rtvHandle.ptr + (UINT64)(nRTVDescriptorSize * m_backBufferIndex) }; 

	// 定位实际的DSV
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = pDSVHeap->GetCPUDescriptorHandleForHeapStart();

}
