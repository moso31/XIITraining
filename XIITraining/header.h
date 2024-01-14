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

class DescriptorAllocator;
class CommittedAllocator;
class PlacedAllocator;

class MeshGenerator;

extern HWND g_hWnd;

class D3D;
extern D3D* g_dx;

extern ComPtr<ID3D12Debug> g_debugController;

// 我的 Win10 版本不是最新的，只支持到 Device8
extern ComPtr<ID3D12Device8> g_pDevice;

// 命令队列相关，目前仅使用一个队列，一个分配器，一个列表
extern ComPtr<ID3D12CommandQueue> g_pCommandQueue; // 命令队列
extern ComPtr<ID3D12CommandAllocator> g_pCommandAllocator; // 命令分配器
extern ComPtr<ID3D12GraphicsCommandList> g_pCommandList; // 命令列表

// 喵喵符分配器
extern DescriptorAllocator* g_pDescriptorAllocator;

// CBuffer分配器
extern CommittedAllocator* g_pCBufferAllocator;

// 纹理分配器
extern PlacedAllocator* g_pTextureAllocator;

// 每帧更新的CB数据
extern CBufferPerFrame g_cbPerFrame;
extern UINT g_cbDataByteOffset; // 记录该数据在 CB 分配器中，分配池 内的 字节偏移量
extern UINT g_cbDataCBufferPageIndex;
extern D3D12_GPU_VIRTUAL_ADDRESS g_cbDataGPUVirtualAddr; // 记录该数据的 GPU 虚拟地址
