#pragma once
#include "header.h"
#include "XAllocator.h"

// why 1000000? see https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
#define DESCRIPTOR_NUM_PER_HEAP_MAXLIMIT 1000000

enum DescriptorType
{
	DescriptorType_CBV,
	DescriptorType_SRV,
	DescriptorType_UAV,
};

class DescriptorAllocator
{
	struct DescriptorPage
	{
		DescriptorPage(UINT dataSize) { freeIntervals.insert({ 0, dataSize - 1 }); }

		std::set<AllocatorRangeInterval> freeIntervals;

		ID3D12DescriptorHeap* data = nullptr;
		DescriptorType type;
	};

public:
	DescriptorAllocator(ID3D12Device* pDevice);

	// �ڶ�����һ�δ�СΪ allocSize �Ŀռ䣬������������
	bool Alloc(DescriptorType type, UINT size, UINT& oPageIdx, UINT& oFirstIdx, D3D12_CPU_DESCRIPTOR_HANDLE& oHandle);

	// �Ƴ� pageIdx ҳ��ģ��� start ��ʼ����Ϊ size ���ڴ��
	void Remove(UINT pageIdx, UINT start, UINT size);

	// ��һ�������������� m_renderHeap �У�����������ring buffer�е�ƫ����
	UINT AppendToRenderHeap(const size_t* cpuHandles, const size_t cpuHandlesSize);

	ID3D12DescriptorHeap* GetRenderHeap() const { return m_renderHeap; }

	const UINT GetRenderHeapDescriptorByteSize() { return m_descriptorByteSize; }

private:
	// �½�һ����������
	void CreateHeap(DescriptorType type, UINT allocSize);

	// ��鵱ǰ���ж�Ӧ���͵Ķ����Ƿ��п��ÿռ�
	bool CheckAllocable(DescriptorType type, UINT allocSize, UINT& oHeapIdx, UINT& oDescriptorIdx);

	// ��ȡ non-shader-visible ������������
	UINT GetDescriptorNum();

private:
	const UINT m_descriptorByteSize;
	ID3D12Device* m_pDevice;

	UINT m_pageNumLimit; // ���allocator������ܷŶ��ٸ�page
	UINT m_eachPageDataNum; // ÿ��page�����ɷŶ��ٸ�data

	// CPU��������ҳ�������ж����������ҳ�档
	// ÿ��ҳֻ����洢��һ���ࣨCBV/SRV/UAV������������
	std::vector<DescriptorPage> m_pages;

	// GPU�������ѡ��ڸ�������Ƴ�һ�� ring buffer��
	ID3D12DescriptorHeap* m_renderHeap;

	// renderHeap ��һ�� ring buffer��ÿ֡��Ҫ���� ring buffer �ϵ�ָ��ƫ��λ��
	UINT m_currentOffset = 0; 
};
