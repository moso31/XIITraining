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

// 我的 Win10 版本不是最新的，只支持到 Device8
ComPtr<ID3D12Device8> g_pDevice;

// 命令队列相关，目前仅使用一个队列，一个分配器，一个列表
ComPtr<ID3D12CommandQueue> g_pCommandQueue; // 命令队列
ComPtr<ID3D12CommandAllocator> g_pCommandAllocator; // 命令分配器
ComPtr<ID3D12GraphicsCommandList> g_pCommandList; // 命令列表

MeshTransformData g_cbObjectData;