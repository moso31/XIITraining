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
	D3D() {}
	~D3D() {}

	void Init();

	void CreateCommandObjects();
	void CreateSwapChain();

	void Release() {}

private:
	ComPtr<ID3D12Device10> m_pDevice;

	// 命令队列相关，目前仅使用一个队列，一个分配器，一个列表
	ComPtr<ID3D12CommandQueue> m_pCommandQueue; // 命令队列
	ComPtr<ID3D12CommandAllocator> m_pCommandAllocator; // 命令分配器
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList; // 命令列表
};

