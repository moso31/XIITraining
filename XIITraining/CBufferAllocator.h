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

	// ����һ��CBV
	template<typename T>
	bool AllocCBV(T& data, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr, UINT& oPageIdx, UINT& oByteOffsetInCBResourcePage)
	{
		size_t blockByteMask = m_blockByteSize - 1;
		UINT dataByteSize = (UINT)((sizeof(T) + blockByteMask) & ~blockByteMask);
		UINT blockSize = dataByteSize / m_blockByteSize; // �ö�CBV��Ҫʹ�ö��ٸ�Block

		UINT oFirstIdx;
		if (Alloc(blockSize, oPageIdx, oFirstIdx))
		{
			auto& pResource = m_pages[oPageIdx].pData;

			oByteOffsetInCBResourcePage = m_blockByteSize * oFirstIdx;

			// ����CBV
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = pResource->GetGPUVirtualAddress() + oByteOffsetInCBResourcePage; // ������������GPU�����ַ
			cbvDesc.SizeInBytes = dataByteSize;

			// ����������
			D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle;
			UINT nouse;
			if (m_pDescriptorAllocator->Alloc(DescriptorType_CBV, 1, nouse, nouse, cbvCpuHandle))
				g_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);

			oGPUVirtualAddr = cbvDesc.BufferLocation;
			return true;
		}

		return false;
	}

	// ����һ����СΪ size ���ڴ��
	// size: Ҫ������ڴ��Ĵ�С
	// oPageIdx: ���䵽��ҳ���±�
	// oFirstIdx: ���䵽��ҳ�еĵ�һ���ڴ����±�
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
					// ����ҵ����ʵĿ����ڴ�
					oPageIdx = i;
					oFirstIdx = space.st;

					if (space.st + size <= space.ed)
						page.freeIntervals.insert({ space.st + size, space.ed });

					page.freeIntervals.erase(space);
					return true;
				}
			}
		}

		// ����Ѿ��ﵽ�����ҳ�����޷��ٷ���
		if (m_pages.size() >= m_pageNumLimit) return false;

		// ���û���ҵ����ʵĿ����ڴ棬��Ҫ�·���һҳ
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

	UINT m_pageNumLimit; // ���allocator������ܷŶ��ٸ�page
	UINT m_eachPageDataNum; // ÿ��page�����ɷŶ��ٸ�data

	UINT m_blockByteSize; // ÿ��page���ÿ��block��ռ���ֽڴ�С����ֵ������256����������

	std::vector<CBufferPage> m_pages;
};
