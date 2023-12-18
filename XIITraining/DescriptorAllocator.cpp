#include "DescriptorAllocator.h"

DescriptorAllocator::DescriptorAllocator(ID3D12Device* pDevice) : 
	m_pDevice(pDevice)
{
}

void DescriptorAllocator::Alloc(DescriptorType type, UINT allocSize, UINT oFirstIndex)
{
	// ������ж����涼û�п��Է��������Ŀռ䣬���½���
	if (CheckAllocable(type, allocSize))
	{

	}
}

void DescriptorAllocator::CreateHeap(DescriptorType type)
{
	DescriptorHeap newHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // cpu heap, Ĭ�� FLAG_NONE = non-shader-visible.
	desc.NodeMask = 0;
	desc.NumDescriptors = 1000000; // why 1000000? see https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // �� allocator ֻ֧�� CBVSRVUAV ��һ������.

	m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap.data));
	newHeap.type = type;

	m_heaps.push_back(newHeap);
}

bool DescriptorAllocator::CheckAllocable(DescriptorType type, UINT allocSize, UINT oHeapIdx, UINT oViewIdx)
{
	for (auto& heap : m_heaps)
	{
		if (heap.type != type) continue;

		const auto& heapDesc = heap.data->GetDesc();
		UINT numViews = heap.data->GetDesc().NumDescriptors;

		auto pHeapHead = heap.data->GetCPUDescriptorHandleForHeapStart();
		pHeapHead.ptr += 
	}
}
