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
	ComPtr<IDXGIFactory4> pDXGIFactory;
	hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&pDXGIFactory));

	// 从 DXGI 工厂中 枚举第一个适配器（默认的显卡）
	ComPtr<IDXGIAdapter1> pAdapter;
	pDXGIFactory->EnumAdapters1(0, &pAdapter);

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
}
