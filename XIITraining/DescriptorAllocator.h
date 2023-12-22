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

	// 记录这个 Descriptor 的内存分配情况，true = 已被分配
	bool allocMap[DESCRIPTOR_NUM_PER_HEAP_MAXLIMIT];

	// 记录 Descriptor 的内存分配数量（allocMap 中有多少个 true）
	UINT allocatedSize;
};

class DescriptorAllocator
{
public:
	DescriptorAllocator(ID3D12Device* pDevice);

	// 在堆里找一段大小为 allocSize 的空间，并分配描述符
	D3D12_CPU_DESCRIPTOR_HANDLE Alloc(DescriptorType type, UINT allocSize);

	// 将一组描述符拷贝到 m_renderHeap 中，并返回其在ring buffer中的偏移量
	UINT AppendToRenderHeap(const size_t* cpuHandles, const size_t cpuHandlesSize);

private:
	// 新建一个描述符堆
	void CreateHeap(DescriptorType type, UINT allocSize);

	// 检查当前所有对应类型的堆中是否还有可用空间
	bool CheckAllocable(DescriptorType type, UINT allocSize, UINT& oHeapIdx, UINT& oDescriptorIdx);

	// 获取 non-shader-visible 描述符的总数
	UINT GetDescriptorNum();

private:
	const UINT m_descriptorSize;
	ID3D12Device* m_pDevice;
	std::vector<DescriptorHeap> m_heaps;

	ID3D12DescriptorHeap* m_renderHeap;
	size_t m_currentOffset = 0;
};
