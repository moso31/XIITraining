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

	// ���������أ�Ŀǰ��ʹ��һ�����У�һ����������һ���б�
	ComPtr<ID3D12CommandQueue> m_pCommandQueue; // �������
	ComPtr<ID3D12CommandAllocator> m_pCommandAllocator; // ���������
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList; // �����б�

	// ������ʹ�õ���������
	ComPtr<ID3D12DescriptorHeap> m_pRTVHeap;
	ComPtr<ID3D12DescriptorHeap> m_pDSVHeap;

	HWND m_hWnd;

	int m_width = 0;
	int m_height = 0;

	// ʼ��ʹ��˫���塣
	int m_swapChainBufferCount = 2;

	// ��ǰ֡��ʹ�õ� backBuffer ��������֡ 0/1 ���档
	int m_backBufferIndex = 0;
};

