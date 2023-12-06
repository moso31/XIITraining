#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <d3dx12.h>

#include <vector>

#include "ShaderStructures.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "DirectXTex/DirectXTex.h"

using namespace Microsoft::WRL;
using namespace DirectX;

extern HWND g_hWnd;

class D3D;
extern D3D* g_dx;

// �ҵ� Win10 �汾�������µģ�ֻ֧�ֵ� Device8
extern ComPtr<ID3D12Device8> g_pDevice;

// ���������أ�Ŀǰ��ʹ��һ�����У�һ����������һ���б�
extern ComPtr<ID3D12CommandQueue> g_pCommandQueue; // �������
extern ComPtr<ID3D12CommandAllocator> g_pCommandAllocator; // ���������
extern ComPtr<ID3D12GraphicsCommandList> g_pCommandList; // �����б�

extern MeshTransformData g_cbObjectData;
