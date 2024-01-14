#pragma once
#include "header.h"
#include "DescriptorAllocator.h"

enum ResourceType
{
	ResourceType_Default,
	ResourceType_Upload,
};

struct CommittedResourcePage
{
	ID3D12Resource* pResource;
	ResourceType type;
};

#define CommittedAllocatorBase XAllocator<CommittedResourcePage>

class CommittedAllocator : public CommittedAllocatorBase
{
public:
	CommittedAllocator(ID3D12Device* pDevice, DescriptorAllocator* pDescriptorAllocator, UINT blockByteSize = 256) : 
		CommittedAllocatorBase(1000000, 100), m_pDevice(pDevice), m_pDescriptorAllocator(pDescriptorAllocator), m_blockByteSize(blockByteSize) {}
	~CommittedAllocator() {}

	//// ����һ��CBV
	//template<typename T>
	//bool AllocCBV(T& data, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr, UINT& oPageIdx, UINT& oByteOffsetInCBResourcePage)
	//{
	//	size_t blockByteMask = m_blockByteSize - 1;
	//	UINT dataByteSize = (UINT)((sizeof(T) + blockByteMask) & ~blockByteMask);
	//	UINT blockSize = dataByteSize / m_blockByteSize; // �ö�CBV��Ҫʹ�ö��ٸ�Block

	//	UINT oFirstIdx;
	//	if (CommittedAllocatorBase::Alloc(blockSize, oPageIdx, oFirstIdx))
	//	{
	//		auto& pResource = m_pages[oPageIdx].data;
	//		oByteOffsetInCBResourcePage = m_blockByteSize * oFirstIdx;

	//		// ����CBV
	//		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	//		cbvDesc.BufferLocation = pResource->GetGPUVirtualAddress() + oByteOffsetInCBResourcePage; // ������������GPU�����ַ
	//		cbvDesc.SizeInBytes = dataByteSize;

	//		// ����������
	//		D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle;
	//		UINT nouse[2];
	//		if (m_pDescriptorAllocator->Alloc(DescriptorType_CBV, 1, nouse[0], nouse[1], cbvCpuHandle))
	//			m_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);

	//		oGPUVirtualAddr = cbvDesc.BufferLocation;
	//		return true;
	//	}

	//	return false;
	//}

	// ����Դ���з���һ���ڴ档
	// byteSize��
	//		Ҫ��������ݵ��ֽڴ�С��ע�ⲻ��ʵ�ʷ�����ֽڴ�С��
	// oGPUVirtualAddr��
	//		��������ݶ�Ӧ��GPU��ַ��
	//		DX12��ȷ������ʱ����Ҫ�ṩGPU��ַ��
	// oPageIdx��
	//		���η�������ݣ���Դ��ҳ��ı�š�
	// oPageByteOffset��
	//		���η�������� ����Դ��ҳ���е�ʵ����ʼ�ֽڡ�
	bool Alloc(UINT byteSize, ResourceType resourceType, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr, UINT& oPageIdx, UINT& oPageByteOffset)
	{
		size_t blockByteMask = m_blockByteSize - 1;
		UINT dataByteSize = (UINT)((byteSize + blockByteMask) & ~blockByteMask);
		UINT blockSize = dataByteSize / m_blockByteSize; // ���alloc��Ҫʹ�ö��ٸ�Block

		UINT oFirstIdx;
		if (CommittedAllocatorBase::Alloc(blockSize, oPageIdx, oFirstIdx))
		{
			auto& pResource = m_pages[oPageIdx].data.pResource;
			oPageByteOffset = m_blockByteSize * oFirstIdx;
			oGPUVirtualAddr = pResource->GetGPUVirtualAddress() + oPageByteOffset;
			return true;
		}

		return false;
	}
	 
	template<typename T>
	void UpdateCBData(T& data, UINT pageIdx, UINT pageByteOffset, UINT pageByteSize)
	{
		auto& pResource = m_pages[pageIdx].data.pResource;

		// ֻ Map Ҫ�޸ĵ���һ�μ���
		D3D12_RANGE mapRange;
		mapRange.Begin = pageByteOffset;
		mapRange.End = pageByteOffset + pageByteSize;

		UINT8* pSrc;
		HRESULT hr = pResource->Map(0, &mapRange, reinterpret_cast<void**>(&pSrc)); 

		UINT8* pDest = pSrc + pageByteOffset;
		memcpy(pDest, &data, sizeof(T));
	}

	void CreateNewPage(CommittedAllocatorBase::Page& newPage) override;

private:
	ID3D12Device* m_pDevice;
	DescriptorAllocator* m_pDescriptorAllocator;

	UINT m_blockByteSize; // ÿ��page���ÿ��block��ռ���ֽڴ�С����ֵ������256����������
};
