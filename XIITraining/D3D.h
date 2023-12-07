#pragma once
#include "header.h"

class Mesh;

class D3DUtil
{
public:
	static UINT CalcBufferViewSize(UINT sizeInBytes);
};

class D3D
{
public:
	D3D(int width, int height) : m_width(width), m_height(height) {}
	~D3D() {}

	void Init();

	void Update();
	void Render();

	void FlushCommandQueue();

	void Release();

	D3D12_CPU_DESCRIPTOR_HANDLE GetSwapChainBackBufferRTV();
	D3D12_CPU_DESCRIPTOR_HANDLE GetSwapChainBackBufferDSV();

private:
	ID3D12Resource* GetSwapChainBackBuffer() const;

	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeap();

	void CreateMyTexture();

	void CreateGlobalConstantBuffers();
	void CreateRootSignature();
	void CreateShaderAndPSO();

	void RenderMeshes();

private:
	ComPtr<IDXGIFactory7> m_pDXGIFactory;
	ComPtr<IDXGISwapChain4> m_pSwapChain;

	ComPtr<ID3D12Fence1> m_pFence;

	// 交换链使用的描述符堆
	ComPtr<ID3D12DescriptorHeap> m_pRTVHeap;
	ComPtr<ID3D12DescriptorHeap> m_pDSVHeap;

	// pObjectCB全局常量缓冲区使用的描述符堆
	ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeapObject;

	int m_width = 0;
	int m_height = 0;

	// 始终使用双缓冲。
	int m_swapChainBufferCount = 2;
	ComPtr<ID3D12Resource> m_pSwapChainRT[2]; // m_swapChainBufferCount == 2
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer;

	// 当前帧所使用的 backBuffer 索引。逐帧 0/1 交替。
	int m_backBufferIndex = 0;

	int m_nRTVDescriptorSize;
	int m_nDSVDescriptorSize;
	int m_nSamplerDescriptorSize;
	int m_nCBSRUAVDescriptorSize;

	UINT64 m_currFenceIdx = 0;

	ComPtr<ID3D12Resource> m_pObjectCBUpload;
	ComPtr<ID3D12RootSignature> m_pRootSignature;
	ComPtr<ID3D12PipelineState> m_pPipelineState;
	Mesh* m_pMesh = nullptr;

	ComPtr<ID3D12Resource> m_pTexture;
	ComPtr<ID3D12Resource> m_pTextureUpload;
};

