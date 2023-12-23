#include "DescriptorAllocator.h"

DescriptorAllocator::DescriptorAllocator(ID3D12Device* pDevice) : 
	m_pDevice(pDevice),
	m_descriptorByteSize(pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
{
	// ����һ�� shader-visible ���������ѣ�������Ⱦǰÿ֡�ύ��
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

	// ��鵱ǰ���ж�Ӧ���͵Ķ����Ƿ��п��ÿռ䣬�еĻ���¼�� �ĸ��ѣ�heapIdx���� �ĸ��������Σ�descriptorIdx��
	if (!CheckAllocable(type, allocSize, heapIdx, descriptorIdx))
	{
		// û�п��ÿռ䣬����һ���µĶ�
		heapIdx = (UINT)m_heaps.size();
		descriptorIdx = 0;
		CreateHeap(type, allocSize);
	}

	// ���� alloc ����ĵ�һ���������� CPU �������� allocSize ��������㷽��ʹ��
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heaps[heapIdx].data->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += descriptorIdx * m_descriptorByteSize;

	return handle;
}

UINT DescriptorAllocator::AppendToRenderHeap(const size_t* cpuHandles, const size_t cpuHandlesSize)
{
	UINT firstOffsetIndex = m_currentOffset;

	for (size_t i = 0; i < cpuHandlesSize; i++)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE srcHandle;
		srcHandle.ptr = cpuHandles[i];

		// �����µ� ring buffer ƫ����
		UINT heapOffset = m_currentOffset * m_descriptorByteSize;
		D3D12_CPU_DESCRIPTOR_HANDLE destHandle = m_renderHeap->GetCPUDescriptorHandleForHeapStart();
		destHandle.ptr += heapOffset;

		// ����������
		m_pDevice->CopyDescriptorsSimple(1, destHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// ����ƫ����
		m_currentOffset = (m_currentOffset + 1) % DESCRIPTOR_NUM_PER_HEAP_MAXLIMIT;
	}

	return firstOffsetIndex;
}

void DescriptorAllocator::CreateHeap(DescriptorType type, UINT allocSize)
{
	DescriptorHeap newHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // cpu heap, Ĭ�� FLAG_NONE = non-shader-visible.
	desc.NodeMask = 0;
	desc.NumDescriptors = DESCRIPTOR_NUM_PER_HEAP_MAXLIMIT; 
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // �� allocator ֻ֧�� CBVSRVUAV ��һ������.

	m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&newHeap.data));
	newHeap.type = type;

	// ��ʼ�� allocMap
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

		// ����������Ѱ�������Ƿ��д�СΪ allocSize �Ŀռ�
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
					// �ҵ��� allocSize ��С�Ŀռ䣬������Ϊ�ѷ���
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
