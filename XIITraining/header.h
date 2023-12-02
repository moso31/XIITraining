#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <DirectXColors.h>
#include <d3dx12.h>

#include <vector>

#include "ShaderStructures.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;

class D3D;

D3D* g_dx = nullptr;

// �ҵ� Win10 �汾�������µģ�ֻ֧�ֵ� Device8
ComPtr<ID3D12Device8> g_pDevice;

// ���������أ�Ŀǰ��ʹ��һ�����У�һ����������һ���б�
ComPtr<ID3D12CommandQueue> g_pCommandQueue; // �������
ComPtr<ID3D12CommandAllocator> g_pCommandAllocator; // ���������
ComPtr<ID3D12GraphicsCommandList> g_pCommandList; // �����б�

MeshTransformData g_cbObjectData;