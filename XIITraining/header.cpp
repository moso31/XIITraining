#include "header.h"

HWND g_hWnd = nullptr;
D3D* g_dx = nullptr;

// 我的 Win10 版本不是最新的，只支持到 Device8
ComPtr<ID3D12Device8> g_pDevice;

// 命令队列相关，目前仅使用一个队列，一个分配器，一个列表
ComPtr<ID3D12CommandQueue> g_pCommandQueue; // 命令队列
ComPtr<ID3D12CommandAllocator> g_pCommandAllocator; // 命令分配器
ComPtr<ID3D12GraphicsCommandList> g_pCommandList; // 命令列表

DescriptorAllocator* g_descriptorAllocator;

MeshTransformData g_cbObjectData;
