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

//D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::Alloc(DescriptorType type, UINT allocSize)
//{
//	UINT heapIdx, descriptorIdx;
//
//	// ��鵱ǰ���ж�Ӧ���͵Ķ����Ƿ��п��ÿռ䣬�еĻ���¼�� �ĸ��ѣ�heapIdx���� �ĸ��������Σ�descriptorIdx��
//	if (!CheckAllocable(type, allocSize, heapIdx, descriptorIdx))
//	{
//		// û�п��ÿռ䣬����һ���µĶ�
//		heapIdx = (UINT)m_heaps.size();
//		descriptorIdx = 0;
//		CreateHeap(type, allocSize);
//	}
//
//	// ���� alloc ����ĵ�һ���������� CPU �������� allocSize ��������㷽��ʹ��
//	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heaps[heapIdx].data->GetCPUDescriptorHandleForHeapStart();
//	handle.ptr += descriptorIdx * m_descriptorByteSize;
//
//	return handle;
//}

// ����һ����СΪ size ���ڴ��
// size: Ҫ������ڴ��Ĵ�С
// oPageIdx: ���䵽��ҳ���±�
// oFirstIdx: ���䵽��ҳ�еĵ�һ���ڴ����±�
bool DescriptorAllocator::Alloc(DescriptorType type, UINT size, UINT& oPageIdx, UINT& oFirstIdx, D3D12_CPU_DESCRIPTOR_HANDLE& oHandle)
{
	if (size > m_eachPageDataNum) return false;

	for (UINT i = 0; i < (UINT)m_pages.size(); i++)
	{
		auto& page = m_pages[i];
		if (page.type != type) continue;

		for (auto& space : page.freeIntervals)
		{
			if (space.ed - space.st + 1 >= size && space.st + size <= m_eachPageDataNum)
			{
				// ����ҵ����ʵĿ����ڴ�
				if (space.st + size <= space.ed)
					page.freeIntervals.insert({ space.st + size, space.ed });

				page.freeIntervals.erase(space);

				oPageIdx = i;
				oFirstIdx = space.st;

				oHandle = page.data->GetCPUDescriptorHandleForHeapStart();
				oHandle.ptr += oFirstIdx * m_descriptorByteSize;
				return true;
			}
		}
	}

	// ����Ѿ��ﵽ�����ҳ�����޷��ٷ���
	if (m_pages.size() >= m_pageNumLimit) return false;

	// ���û���ҵ����ʵĿ����ڴ棬��Ҫ�·���һҳ
	auto& newPage = m_pages.emplace_back(m_eachPageDataNum);
	oPageIdx = (UINT)m_pages.size() - 1;
	oFirstIdx = 0;
	oHandle = newPage.data->GetCPUDescriptorHandleForHeapStart();
	return true;
}

void DescriptorAllocator::Remove(UINT pageIdx, UINT start, UINT size)
{
	auto& freeIntervals = m_pages[pageIdx].freeIntervals;

	UINT end = min(start + size - 1, m_eachPageDataNum - 1);

	AllocatorRangeInterval adjust(start, end);
	std::set<AllocatorRangeInterval> removing;
	for (auto& space : freeIntervals)
	{
		bool bCombine = false;
		if (space.st >= start && space.ed <= end)
		{
			// ��� space ���Ӽ���ɾ��
			removing.insert(space);
		}
		else if (space.st <= end && start <= space.ed)
		{
			// ��� space �ǽ������ϲ�
			removing.insert(space);
			bCombine = true;
		}
		else if (space.st < start || space.ed > end)
		{
			// ��� space �Ǹ�����ʲô������
		}
		else bCombine = true;

		if (bCombine)
		{
			adjust.st = min(adjust.st, space.st);
			adjust.ed = max(adjust.ed, space.ed);
		}
	}

	for (auto& space : removing) freeIntervals.erase(space);

	// ��� adjust �� m_freeInterval �γ����ţ���Ҫ�ٺϲ�һ�Ρ�
	removing.clear();
	for (auto& space : freeIntervals)
	{
		if (space.st == adjust.ed + 1)
		{
			adjust.ed = space.ed;
			removing.insert(space);
		}
		else if (space.ed == adjust.st - 1)
		{
			adjust.st = space.st;
			removing.insert(space);
		}
	}

	freeIntervals.insert(adjust);
	for (auto& space : removing) freeIntervals.erase(space);
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
