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

	//// 分配一段CBV
	//template<typename T>
	//bool AllocCBV(T& data, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr, UINT& oPageIdx, UINT& oByteOffsetInCBResourcePage)
	//{
	//	size_t blockByteMask = m_blockByteSize - 1;
	//	UINT dataByteSize = (UINT)((sizeof(T) + blockByteMask) & ~blockByteMask);
	//	UINT blockSize = dataByteSize / m_blockByteSize; // 该段CBV需要使用多少个Block

	//	UINT oFirstIdx;
	//	if (CommittedAllocatorBase::Alloc(blockSize, oPageIdx, oFirstIdx))
	//	{
	//		auto& pResource = m_pages[oPageIdx].data;
	//		oByteOffsetInCBResourcePage = m_blockByteSize * oFirstIdx;

	//		// 创建CBV
	//		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	//		cbvDesc.BufferLocation = pResource->GetGPUVirtualAddress() + oByteOffsetInCBResourcePage; // 常量缓冲区的GPU虚拟地址
	//		cbvDesc.SizeInBytes = dataByteSize;

	//		// 分配描述符
	//		D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle;
	//		UINT nouse[2];
	//		if (m_pDescriptorAllocator->Alloc(DescriptorType_CBV, 1, nouse[0], nouse[1], cbvCpuHandle))
	//			m_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);

	//		oGPUVirtualAddr = cbvDesc.BufferLocation;
	//		return true;
	//	}

	//	return false;
	//}

	// 在资源池中分配一段内存。
	// byteSize：
	//		要分配的数据的字节大小（注意不是实际分配的字节大小）
	// oGPUVirtualAddr：
	//		分配的数据对应的GPU地址。
	//		DX12明确根参数时，需要提供GPU地址。
	// oPageIdx：
	//		本次分配的数据，资源池页面的编号。
	// oPageByteOffset：
	//		本次分配的数据 在资源池页面中的实际起始字节。
	bool Alloc(UINT byteSize, ResourceType resourceType, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr, UINT& oPageIdx, UINT& oPageByteOffset)
	{
		size_t blockByteMask = m_blockByteSize - 1;
		UINT dataByteSize = (UINT)((byteSize + blockByteMask) & ~blockByteMask);
		UINT blockSize = dataByteSize / m_blockByteSize; // 这次alloc需要使用多少个Block

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

		// 只 Map 要修改的那一段即可
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

	UINT m_blockByteSize; // 每个page里的每个block所占的字节大小（该值必须是256的整数倍）
};
