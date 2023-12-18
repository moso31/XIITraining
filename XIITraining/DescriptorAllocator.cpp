#include "DescriptorAllocator.h"

DescriptorAllocator::DescriptorAllocator(ID3D12Device* pDevice) : 
	m_pDevice(pDevice)
{
}

void DescriptorAllocator::AllocSRVHeap(UINT allocSize, UINT oFirstIndex)
{
	// 如果所有堆里面都没有可以放描述符的空间，就新建堆
	if (m_srvHeaps.empty())
	{
		CreateHeap(DescriptorType_SRV);
	}
}

void DescriptorAllocator::CreateHeap(DescriptorType type)
{
	DescriptorHeap newHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // cpu heap, 默认 FLAG_NONE = non-shader-visible.
	desc.NodeMask = 0;
	desc.NumDescriptors = 1000000; // why 1000000? see https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // 此 allocator 只支持 CBVSRVUAV 这一种类型.

	m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap.heap));
	newHeap.heapType = type;

	switch (type)
	{
	case DescriptorType_CBV: m_cbvHeaps.push_back(newHeap); break;
	case DescriptorType_SRV: m_srvHeaps.push_back(newHeap); break;
	case DescriptorType_UAV: break;
	default: break;
	}
}
