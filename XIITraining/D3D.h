#pragma once
#include "header.h"

class Mesh;
class Texture;
class Material;

class D3D
{
public:
	D3D(int width, int height) : m_width(width), m_height(height) {}
	~D3D() {}

	void Init();

	// Prepare 的职责：
	// 按材质遍历，对每个材质，按材质shader代码中所记录的描述符顺序，通过descriptorAllocator，在shader-visible堆中追加同样数量的描述符集，并记录在shader-visible堆中生成的第一个描述符的偏移量
	void Prepare();
	void Draw();

	void FlushCommandQueue();

	void Release();

	D3D12_CPU_DESCRIPTOR_HANDLE GetSwapChainBackBufferRTV();
	D3D12_CPU_DESCRIPTOR_HANDLE GetSwapChainBackBufferDSV();

private:
	ID3D12Resource* GetSwapChainBackBuffer() const;

	void CreateCommandObjects();
	void CreateSwapChain();
	void CreateDescriptorHeap();

	void CreateCBufferPerFrame();

	void Update();
	void RenderMeshes();

private:
	ComPtr<IDXGIFactory7> m_pDXGIFactory;
	ComPtr<IDXGISwapChain4> m_pSwapChain;

	ComPtr<ID3D12Fence1> m_pFence;

	// 交换链使用的描述符堆
	ComPtr<ID3D12DescriptorHeap> m_pRTVHeap;
	ComPtr<ID3D12DescriptorHeap> m_pDSVHeap;

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

	ComPtr<ID3D12Resource> m_pCBPerFrame;

	Mesh* m_pMesh = nullptr;
	Mesh* m_pMeshCube = nullptr;

	std::vector<Material*> m_pMaterials;

	Texture* m_pTextureBox = nullptr;
	Texture* m_pTextureCubeMap = nullptr;

	// 每帧更新的CB数据
	CBufferPerFrame m_cbPerFrame;
	UINT cbDataByteOffset; // 记录该数据在 CB 分配器中，分配池 内的 字节偏移量
};

