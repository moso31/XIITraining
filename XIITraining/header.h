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

// 我的 Win10 版本不是最新的，只支持到 Device8
extern ComPtr<ID3D12Device8> g_pDevice;

// 命令队列相关，目前仅使用一个队列，一个分配器，一个列表
extern ComPtr<ID3D12CommandQueue> g_pCommandQueue; // 命令队列
extern ComPtr<ID3D12CommandAllocator> g_pCommandAllocator; // 命令分配器
extern ComPtr<ID3D12GraphicsCommandList> g_pCommandList; // 命令列表

extern MeshTransformData g_cbObjectData;
