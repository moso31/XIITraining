#pragma once
#include "header.h"
#include "DescriptorAllocator.h"

#define ResourceAllocator XAllocator<ID3D12Resource*>

class CommittedAllocator : public ResourceAllocator
{
public:
	CommittedAllocator(ID3D12Device* pDevice, DescriptorAllocator* pDescriptorAllocator, UINT blockByteSize = 256) : 
		ResourceAllocator(1000000, 100), m_pDevice(pDevice), m_pDescriptorAllocator(pDescriptorAllocator), m_blockByteSize(blockByteSize) {}
	~CommittedAllocator() {}

	// ����һ��CBV
	template<typename T>
	bool AllocCBV(T& data, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr, UINT& oPageIdx, UINT& oByteOffsetInCBResourcePage)
	{
		size_t blockByteMask = m_blockByteSize - 1;
		UINT dataByteSize = (UINT)((sizeof(T) + blockByteMask) & ~blockByteMask);
		UINT blockSize = dataByteSize / m_blockByteSize; // �ö�CBV��Ҫʹ�ö��ٸ�Block

		UINT oFirstIdx;
		if (ResourceAllocator::Alloc(blockSize, oPageIdx, oFirstIdx))
		{
			auto& pResource = m_pages[oPageIdx].data;

			oByteOffsetInCBResourcePage = m_blockByteSize * oFirstIdx;

			// ����CBV
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = pResource->GetGPUVirtualAddress() + oByteOffsetInCBResourcePage; // ������������GPU�����ַ
			cbvDesc.SizeInBytes = dataByteSize;

			// ����������
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

	void CreateNewPage(ResourceAllocator::Page& newPage) override;

private:
	ID3D12Device* m_pDevice;
	DescriptorAllocator* m_pDescriptorAllocator;

	UINT m_blockByteSize; // ÿ��page���ÿ��block��ռ���ֽڴ�С����ֵ������256����������
};
