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

	// ���������أ�Ŀǰ��ʹ��һ�����У�һ����������һ���б�
	ComPtr<ID3D12CommandQueue> m_pCommandQueue; // �������
	ComPtr<ID3D12CommandAllocator> m_pCommandAllocator; // ���������
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList; // �����б�
};

