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

	// 在堆里找一段大小为 allocSize 的空间，并分配描述符
	bool Alloc(DescriptorType type, UINT size, UINT& oPageIdx, UINT& oFirstIdx, D3D12_CPU_DESCRIPTOR_HANDLE& oHandle);

	// 移除 pageIdx 页面的，从 start 开始长度为 size 的内存块
	void Remove(UINT pageIdx, UINT start, UINT size);

	// 将一组描述符拷贝到 m_renderHeap 中，并返回其在ring buffer中的偏移量
	UINT AppendToRenderHeap(const size_t* cpuHandles, const size_t cpuHandlesSize);

	ID3D12DescriptorHeap* GetRenderHeap() const { return m_renderHeap; }

	const UINT GetRenderHeapDescriptorByteSize() { return m_descriptorByteSize; }

private:
	// 新建一个描述符堆
	void CreateHeap(DescriptorType type, UINT allocSize);

	// 检查当前所有对应类型的堆中是否还有可用空间
	bool CheckAllocable(DescriptorType type, UINT allocSize, UINT& oHeapIdx, UINT& oDescriptorIdx);

	// 获取 non-shader-visible 描述符的总数
	UINT GetDescriptorNum();

private:
	const UINT m_descriptorByteSize;
	ID3D12Device* m_pDevice;

	UINT m_pageNumLimit; // 这个allocator中最多能放多少个page
	UINT m_eachPageDataNum; // 每个page中最多可放多少个data

	// CPU描述符堆页，允许有多个描述符堆页面。
	// 每个页只允许存储单一种类（CBV/SRV/UAV）的描述符。
	std::vector<DescriptorPage> m_pages;

	// GPU描述符堆。在概念上设计成一个 ring buffer。
	ID3D12DescriptorHeap* m_renderHeap;

	// renderHeap 是一个 ring buffer，每帧都要更新 ring buffer 上的指针偏移位置
	UINT m_currentOffset = 0; 
};
