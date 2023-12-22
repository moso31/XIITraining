#pragma once
#include "header.h"

// why 1000000? see https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
#define DESCRIPTOR_NUM_PER_HEAP_MAXLIMIT 1000000

enum DescriptorType
{
	DescriptorType_CBV,
	DescriptorType_SRV,
	DescriptorType_UAV,
};

struct DescriptorAllocRange
{
	UINT heapIdx;
	UINT start;
	UINT length;
};

struct DescriptorHeap
{
	ID3D12DescriptorHeap* data;
	DescriptorType type;

	// ��¼��� Descriptor ���ڴ���������true = �ѱ�����
	bool allocMap[DESCRIPTOR_NUM_PER_HEAP_MAXLIMIT];

	// ��¼ Descriptor ���ڴ����������allocMap ���ж��ٸ� true��
	UINT allocatedSize;
};

class DescriptorAllocator
{
public:
	DescriptorAllocator(ID3D12Device* pDevice);

	// �ڶ�����һ�δ�СΪ allocSize �Ŀռ䣬������������
	D3D12_CPU_DESCRIPTOR_HANDLE Alloc(DescriptorType type, UINT allocSize);

	// ��һ�������������� m_renderHeap �У�����������ring buffer�е�ƫ����
	UINT AppendToRenderHeap(const size_t* cpuHandles, const size_t cpuHandlesSize);

private:
	// �½�һ����������
	void CreateHeap(DescriptorType type, UINT allocSize);

	// ��鵱ǰ���ж�Ӧ���͵Ķ����Ƿ��п��ÿռ�
	bool CheckAllocable(DescriptorType type, UINT allocSize, UINT& oHeapIdx, UINT& oDescriptorIdx);

	// ��ȡ non-shader-visible ������������
	UINT GetDescriptorNum();

private:
	const UINT m_descriptorSize;
	ID3D12Device* m_pDevice;
	std::vector<DescriptorHeap> m_heaps;

	ID3D12DescriptorHeap* m_renderHeap;
	size_t m_currentOffset = 0;
};
