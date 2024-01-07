#include "header.h"

HWND g_hWnd = nullptr;
D3D* g_dx = nullptr;

ComPtr<ID3D12Debug> g_debugController;
ComPtr<ID3D12Device8> g_pDevice;

ComPtr<ID3D12CommandQueue> g_pCommandQueue; 
ComPtr<ID3D12CommandAllocator> g_pCommandAllocator; 
ComPtr<ID3D12GraphicsCommandList> g_pCommandList; 

DescriptorAllocator* g_pDescriptorAllocator;
CommittedAllocator* g_pCBufferAllocator;
PlacedAllocator* g_pTextureAllocator;

CBufferPerFrame g_cbPerFrame;
UINT g_cbDataByteOffset; 
UINT g_cbDataCBufferPageIndex;
D3D12_GPU_VIRTUAL_ADDRESS g_cbDataGPUVirtualAddr; 
