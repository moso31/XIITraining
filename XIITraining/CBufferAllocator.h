#pragma once
#include "header.h"
#include "DescriptorAllocator.h"

class CBufferAllocator
{
	struct CBufferPage
	{
		CBufferPage(UINT dataSize) { freeIntervals.insert({ 0, dataSize - 1 }); }
		std::set<AllocatorRangeInterval> freeIntervals;

		ID3D12Resource* pData;
	};

public:
	CBufferAllocator(ID3D12Device* pDevice, DescriptorAllocator* pDescriptorAllocator, UINT blockByteSize = 256) : 
		m_pDevice(pDevice), m_pDescriptorAllocator(pDescriptorAllocator), m_blockByteSize(blockByteSize), m_eachPageDataNum(1000000), m_pageNumLimit(100) {}
	~CBufferAllocator() {}

	// 分配一段CBV
	template<typename T>
	bool AllocCBV(T& data, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr, UINT& oPageIdx, UINT& oByteOffsetInCBResourcePage)
	{
		size_t blockByteMask = m_blockByteSize - 1;
		UINT dataByteSize = (UINT)((sizeof(T) + blockByteMask) & ~blockByteMask);
		UINT blockSize = dataByteSize / m_blockByteSize; // 该段CBV需要使用多少个Block

		UINT oFirstIdx;
		if (Alloc(blockSize, oPageIdx, oFirstIdx))
		{
			auto& pResource = m_pages[oPageIdx].pData;

			oByteOffsetInCBResourcePage = m_blockByteSize * oFirstIdx;

			// 创建CBV
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = pResource->GetGPUVirtualAddress() + oByteOffsetInCBResourcePage; // 常量缓冲区的GPU虚拟地址
			cbvDesc.SizeInBytes = dataByteSize;

			// 分配描述符
			D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle;
			UINT nouse;
			if (m_pDescriptorAllocator->Alloc(DescriptorType_CBV, 1, nouse, nouse, cbvCpuHandle))
				g_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);

			oGPUVirtualAddr = cbvDesc.BufferLocation;
			return true;
		}

		return false;
	}

	// 分配一个大小为 size 的内存块
	// size: 要分配的内存块的大小
	// oPageIdx: 分配到的页的下标
	// oFirstIdx: 分配到的页中的第一个内存块的下标
	bool Alloc(UINT size, UINT& oPageIdx, UINT& oFirstIdx)
	{
		if (size > m_eachPageDataNum) return false;

		for (UINT i = 0; i < (UINT)m_pages.size(); i++)
		{
			auto& page = m_pages[i];

			for (auto& space : page.freeIntervals)
			{
				if (space.ed - space.st + 1 >= size && space.st + size <= m_eachPageDataNum)
				{
					// 如果找到合适的空闲内存
					oPageIdx = i;
					oFirstIdx = space.st;

					if (space.st + size <= space.ed)
						page.freeIntervals.insert({ space.st + size, space.ed });

					page.freeIntervals.erase(space);
					return true;
				}
			}
		}

		// 如果已经达到了最大页数，无法再分配
		if (m_pages.size() >= m_pageNumLimit) return false;

		// 如果没有找到合适的空闲内存，需要新分配一页
		auto& newPage = m_pages.emplace_back(m_eachPageDataNum);
		newPage.freeIntervals.clear();
		newPage.freeIntervals.insert({ size, m_eachPageDataNum - 1 });
		CreateResourcePage(newPage);
		oPageIdx = (UINT)m_pages.size() - 1;
		oFirstIdx = 0;
		return true;
	}

	void CreateResourcePage(CBufferPage& newPage);
	 
	template<typename T>
	void UpdateCBData(T& data, UINT pageIdx, UINT cbDataByteOffset)
	{
		auto& pResource = m_pages[pageIdx].pData;

		UINT8* pSrc;
		HRESULT hr = pResource->Map(0, nullptr, reinterpret_cast<void**>(&pSrc));

		UINT8* pDest = pSrc + cbDataByteOffset;
		memcpy(pDest, &data, sizeof(T));
	}

private:
	ID3D12Device* m_pDevice;
	DescriptorAllocator* m_pDescriptorAllocator;

	UINT m_pageNumLimit; // 这个allocator中最多能放多少个page
	UINT m_eachPageDataNum; // 每个page中最多可放多少个data

	UINT m_blockByteSize; // 每个page里的每个block所占的字节大小（该值必须是256的整数倍）

	std::vector<CBufferPage> m_pages;
};
