#include "header.h"

HWND g_hWnd = nullptr;
D3D* g_dx = nullptr;

// �ҵ� Win10 �汾�������µģ�ֻ֧�ֵ� Device8
ComPtr<ID3D12Device8> g_pDevice;

// ���������أ�Ŀǰ��ʹ��һ�����У�һ����������һ���б�
ComPtr<ID3D12CommandQueue> g_pCommandQueue; // �������
ComPtr<ID3D12CommandAllocator> g_pCommandAllocator; // ���������
ComPtr<ID3D12GraphicsCommandList> g_pCommandList; // �����б�

DescriptorAllocator* g_descriptorAllocator;

MeshTransformData g_cbObjectData;
