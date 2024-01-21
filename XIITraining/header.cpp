#include "header.h"

HWND g_hWnd = nullptr;
D3D* g_dx = nullptr;

ComPtr<ID3D12Debug> g_debugController;
ComPtr<ID3D12Device8> g_pDevice;

ComPtr<ID3D12CommandQueue> g_pCommandQueue; 
ComPtr<ID3D12CommandAllocator> g_pCommandAllocator[FRAME_BUFFER_NUM];
ComPtr<ID3D12GraphicsCommandList> g_pCommandList; 

DescriptorAllocator* g_pDescriptorAllocator;
CommittedAllocator* g_pCBufferAllocator;
PlacedAllocator* g_pTextureAllocator;

MultiFrame<XAllocatorData<CBufferPerFrameData>> g_cbPerFrame;
