#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

using namespace Microsoft::WRL;

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class D3D
{
public:
	D3D(int width, int height, HWND hWnd) : m_width(width), m_height(height), m_hWnd(hWnd) {}
	~D3D() {}

	void Init();

	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeap();

	void Render();

	void Release() {}

private:
	ComPtr<ID3D12Device10> m_pDevice;
	ComPtr<IDXGIFactory7> m_pDXGIFactory;
	ComPtr<IDXGISwapChain4> m_pSwapChain;

	ComPtr<ID3D12Fence1> m_pFence;

	// 命令队列相关，目前仅使用一个队列，一个分配器，一个列表
	ComPtr<ID3D12CommandQueue> m_pCommandQueue; // 命令队列
	ComPtr<ID3D12CommandAllocator> m_pCommandAllocator; // 命令分配器
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList; // 命令列表

	// 交换链使用的描述符堆
	ComPtr<ID3D12DescriptorHeap> m_pRTVHeap;
	ComPtr<ID3D12DescriptorHeap> m_pDSVHeap;

	HWND m_hWnd;

	int m_width = 0;
	int m_height = 0;

	// 始终使用双缓冲。
	int m_swapChainBufferCount = 2;

	// 当前帧所使用的 backBuffer 索引。逐帧 0/1 交替。
	int m_backBufferIndex = 0;
};

