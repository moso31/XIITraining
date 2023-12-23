#pragma once
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

#include "ShaderStructures.h"

#include "DirectXTex/DirectXTex.h"

using namespace Microsoft::WRL;
using namespace DirectX;

class DescriptorAllocator;
class CBufferAllocator;

extern HWND g_hWnd;

class D3D;
extern D3D* g_dx;

// �ҵ� Win10 �汾�������µģ�ֻ֧�ֵ� Device8
extern ComPtr<ID3D12Device8> g_pDevice;

// ���������أ�Ŀǰ��ʹ��һ�����У�һ����������һ���б�
extern ComPtr<ID3D12CommandQueue> g_pCommandQueue; // �������
extern ComPtr<ID3D12CommandAllocator> g_pCommandAllocator; // ���������
extern ComPtr<ID3D12GraphicsCommandList> g_pCommandList; // �����б�

// ������������
extern DescriptorAllocator* g_pDescriptorAllocator;

// CBuffer������
extern CBufferAllocator* g_pCBufferAllocator;

// ÿ֡���µ�CB����
extern CBufferPerFrame g_cbPerFrame;
extern UINT g_cbDataByteOffset; // ��¼�������� CB �������У������ �ڵ� �ֽ�ƫ����
extern D3D12_GPU_VIRTUAL_ADDRESS g_cbDataGPUVirtualAddr; // ��¼�����ݵ� GPU �����ַ
