#pragma once
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>

#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <random>

#include "ShaderStructures.h"

#include "DirectXTex/DirectXTex.h"

using namespace Microsoft::WRL;
using namespace DirectX;

#define FRAME_BUFFER_NUM 8

template <typename T>
class MultiFrame
{
public:
    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }

	void Reset(const T& val) { for (int i = 0; i < FRAME_BUFFER_NUM; i++) data[i] = val; }

protected:
    T data[FRAME_BUFFER_NUM];
};

template <typename T>
struct XAllocatorData
{
	virtual UINT DataByteSize() { return sizeof(T); }

	T data;
	UINT pageIndex; // ��¼�������� XAllocator ��ҳ������
	UINT pageByteOffset; // ��¼�������� XAllocator ��ҳ����ֽ�ƫ����
	D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAddr; // ��¼�����ݵ� GPU �����ַ
};

class DescriptorAllocator;
class CommittedAllocator;
class PlacedAllocator;

class MeshGenerator;

extern HWND g_hWnd;

class D3D;
extern D3D* g_dx;

extern ComPtr<ID3D12Debug> g_debugController;

// �ҵ� Win10 �汾�������µģ�ֻ֧�ֵ� Device8
extern ComPtr<ID3D12Device8> g_pDevice;

// ���������أ�Ŀǰ��ʹ��һ�����У�һ����������һ���б�
extern ComPtr<ID3D12CommandQueue> g_pCommandQueue; // �������
extern ComPtr<ID3D12CommandAllocator> g_pCommandAllocator[FRAME_BUFFER_NUM]; // ���������
extern ComPtr<ID3D12GraphicsCommandList> g_pCommandList; // �����б�

// ������������
extern DescriptorAllocator* g_pDescriptorAllocator;

// CBuffer������
extern CommittedAllocator* g_pCBufferAllocator;

// ���������
extern PlacedAllocator* g_pTextureAllocator;

// ÿ֡���µ�CB����
extern MultiFrame<XAllocatorData<CBufferPerFrameData>> g_cbPerFrame;
