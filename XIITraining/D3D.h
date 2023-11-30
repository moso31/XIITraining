#pragma once
#include "header.h"

class D3D
{
public:
	D3D(int width, int height, HWND hWnd) : m_width(width), m_height(height), m_hWnd(hWnd) {}
	~D3D() {}

	void Init();

	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeap();

	D3D12_CPU_DESCRIPTOR_HANDLE GetSwapChainBackBufferRTV();
	D3D12_CPU_DESCRIPTOR_HANDLE GetSwapChainBackBufferDSV();

	void Prepare();
	void Render();

	void FlushCommandQueue();

	void Release() {}

private:
	ID3D12Resource* GetSwapChainBackBuffer() const;

private:
	ComPtr<IDXGIFactory7> m_pDXGIFactory;
	ComPtr<IDXGISwapChain4> m_pSwapChain;

	ComPtr<ID3D12Fence1> m_pFence;

	// ������ʹ�õ���������
	ComPtr<ID3D12DescriptorHeap> m_pRTVHeap;
	ComPtr<ID3D12DescriptorHeap> m_pDSVHeap;

	HWND m_hWnd;

	int m_width = 0;
	int m_height = 0;

	// ʼ��ʹ��˫���塣
	int m_swapChainBufferCount = 2;
	ComPtr<ID3D12Resource> m_pSwapChainRT[2]; // m_swapChainBufferCount == 2
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer;

	// ��ǰ֡��ʹ�õ� backBuffer ��������֡ 0/1 ���档
	int m_backBufferIndex = 0;

	int m_nRTVDescriptorSize;
	int m_nDSVDescriptorSize;
	int m_nSamplerDescriptorSize;
	int m_nCBSRUAVDescriptorSize;

	UINT64 m_currFenceIdx = 0;
};

