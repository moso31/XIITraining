#pragma once
#include "header.h"
#include "DescriptorAllocator.h"

#define CommittedAllocatorBase XAllocator<ID3D12Resource*>

class CommittedAllocator : public CommittedAllocatorBase
{
public:
	CommittedAllocator(ID3D12Device* pDevice, DescriptorAllocator* pDescriptorAllocator, UINT blockByteSize = 256) : 
		CommittedAllocatorBase(1000000, 100), m_pDevice(pDevice), m_pDescriptorAllocator(pDescriptorAllocator), m_blockByteSize(blockByteSize) {}
	~CommittedAllocator() {}

	// 分配一段CBV
	template<typename T>
	bool AllocCBV(T& data, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr, UINT& oPageIdx, UINT& oByteOffsetInCBResourcePage)
	{
		size_t blockByteMask = m_blockByteSize - 1;
		UINT dataByteSize = (UINT)((sizeof(T) + blockByteMask) & ~blockByteMask);
		UINT blockSize = dataByteSize / m_blockByteSize; // 该段CBV需要使用多少个Block

		UINT oFirstIdx;
		if (CommittedAllocatorBase::Alloc(blockSize, oPageIdx, oFirstIdx))
		{
			auto& pResource = m_pages[oPageIdx].data;
			oByteOffsetInCBResourcePage = m_blockByteSize * oFirstIdx;

			// 创建CBV
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = pResource->GetGPUVirtualAddress() + oByteOffsetInCBResourcePage; // 常量缓冲区的GPU虚拟地址
			cbvDesc.SizeInBytes = dataByteSize;

			// 分配描述符
			D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle;
			UINT nouse[2];
			if (m_pDescriptorAllocator->Alloc(DescriptorType_CBV, 1, nouse[0], nouse[1], cbvCpuHandle))
				m_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);

			oGPUVirtualAddr = cbvDesc.BufferLocation;
			return true;
		}

		return false;
	}
	 
	template<typename T>
	void UpdateCBData(T& data, UINT pageIdx, UINT cbDataByteOffset)
	{
		auto& pResource = m_pages[pageIdx].data;

		UINT8* pSrc;
		HRESULT hr = pResource->Map(0, nullptr, reinterpret_cast<void**>(&pSrc));

		UINT8* pDest = pSrc + cbDataByteOffset;
		memcpy(pDest, &data, sizeof(T));
	}

	void CreateNewPage(CommittedAllocatorBase::Page& newPage) override;

private:
	ID3D12Device* m_pDevice;
	DescriptorAllocator* m_pDescriptorAllocator;

	UINT m_blockByteSize; // 每个page里的每个block所占的字节大小（该值必须是256的整数倍）
};
