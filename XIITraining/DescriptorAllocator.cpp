#include "DescriptorAllocator.h"

DescriptorAllocator::DescriptorAllocator(ID3D12Device* pDevice) : 
	m_pDevice(pDevice),
	m_descriptorSize(pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
{
	// 创建一个 shader-visible 的描述符堆，用于渲染前每帧提交。
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	desc.NumDescriptors = DESCRIPTOR_NUM_PER_HEAP_MAXLIMIT;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_renderHeap));
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::Alloc(DescriptorType type, UINT allocSize)
{
	UINT heapIdx, descriptorIdx;

	// 检查当前所有对应类型的堆中是否还有可用空间，有的话记录是 哪个堆（heapIdx）的 哪个描述符段（descriptorIdx）
	if (!CheckAllocable(type, allocSize, heapIdx, descriptorIdx))
	{
		// 没有可用空间，创建一个新的堆
		heapIdx = (UINT)m_heaps.size();
		descriptorIdx = 0;
		CreateHeap(type, allocSize);
	}

	// 返回 alloc 分配的第一个描述符的 CPU 句柄，配合 allocSize 即可让外层方法使用
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heaps[heapIdx].data->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += descriptorIdx * m_descriptorSize;

	return handle;
}

void DescriptorAllocator::AppendToRenderHeap(const size_t* cpuHandles, const size_t cpuHandlesSize)
{
	for (size_t i = 0; i < cpuHandlesSize; i++)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE srcHandle;
		srcHandle.ptr = cpuHandles[i];

		// 计算新的 ring buffer 偏移量
		size_t heapOffset = m_currentOffset * m_descriptorSize;
		D3D12_CPU_DESCRIPTOR_HANDLE destHandle = m_renderHeap->GetCPUDescriptorHandleForHeapStart();
		destHandle.ptr += heapOffset;

		// 拷贝描述符
		m_pDevice->CopyDescriptorsSimple(1, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// 更新偏移量
		m_currentOffset = (m_currentOffset + 1) % DESCRIPTOR_NUM_PER_HEAP_MAXLIMIT;
	}
}

void DescriptorAllocator::CreateHeap(DescriptorType type, UINT allocSize)
{
	DescriptorHeap newHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // cpu heap, 默认 FLAG_NONE = non-shader-visible.
	desc.NodeMask = 0;
	desc.NumDescriptors = DESCRIPTOR_NUM_PER_HEAP_MAXLIMIT; 
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // 此 allocator 只支持 CBVSRVUAV 这一种类型.

	m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap.data));
	newHeap.type = type;

	// 初始化 allocMap
	memset(newHeap.allocMap, 0, sizeof(bool) * DESCRIPTOR_NUM_PER_HEAP_MAXLIMIT);
	memset(newHeap.allocMap, 1, sizeof(bool) * allocSize);
	newHeap.allocatedSize = allocSize;

	m_heaps.push_back(newHeap);
}

bool DescriptorAllocator::CheckAllocable(DescriptorType type, UINT allocSize, UINT& oHeapIdx, UINT& oDescriptorIdx)
{
	for (UINT heapIdx = 0; heapIdx < m_heaps.size(); heapIdx++)
	{
		auto& heap = m_heaps[heapIdx];
		if (heap.type != type) continue;

		const auto& heapDesc = heap.data->GetDesc();

		// 遍历整个堆寻找其中是否还有大小为 allocSize 的空间
		auto p = heap.data->GetCPUDescriptorHandleForHeapStart();
		UINT idx = 0;
		while (idx < heapDesc.NumDescriptors)
		{
			if (!heap.allocMap[idx])
			{
				UINT i = idx;
				while (i - idx != allocSize && !heap.allocMap[i]) i++;
				if (i - idx == allocSize)
				{
					// 找到了 allocSize 大小的空间，将其标记为已分配
					for (UINT j = idx; j < i; j++) heap.allocMap[j] = true;
					oHeapIdx = heapIdx;
					oDescriptorIdx = idx;
					heap.allocatedSize += allocSize;
					return true;
				}
				else idx = i + 1;
			}
			else idx++;
		}
	}
	return false;
}

UINT DescriptorAllocator::GetDescriptorNum()
{
	UINT result = 0;
	for (auto& heap : m_heaps) result += heap.allocatedSize;
	return result;
}
