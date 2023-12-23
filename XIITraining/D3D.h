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

	// Prepare ��ְ��
	// �����ʱ�������ÿ�����ʣ�������shader����������¼��������˳��ͨ��descriptorAllocator����shader-visible����׷��ͬ����������������������¼��shader-visible�������ɵĵ�һ����������ƫ����
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

	// ������ʹ�õ���������
	ComPtr<ID3D12DescriptorHeap> m_pRTVHeap;
	ComPtr<ID3D12DescriptorHeap> m_pDSVHeap;

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

	ComPtr<ID3D12Resource> m_pCBPerFrame;

	Mesh* m_pMesh = nullptr;
	Mesh* m_pMeshCube = nullptr;

	std::vector<Material*> m_pMaterials;

	Texture* m_pTextureBox = nullptr;
	Texture* m_pTextureCubeMap = nullptr;

	// ÿ֡���µ�CB����
	CBufferPerFrame m_cbPerFrame;
	UINT cbDataByteOffset; // ��¼�������� CB �������У������ �ڵ� �ֽ�ƫ����
};

