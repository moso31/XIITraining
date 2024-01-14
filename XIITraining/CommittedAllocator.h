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
	bool Alloc(UINT byteSize, ResourceType resourceType, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr, UINT& oPageIdx, UINT& oPageByteOffset);
	 
	void UpdateCBData(void* data, UINT dataSize, UINT pageIdx, UINT pageByteOffset);

	void CreateNewPage(CommittedAllocatorBase::Page& newPage) override;

private:
	ID3D12Device* m_pDevice;
	DescriptorAllocator* m_pDescriptorAllocator;

	UINT m_blockByteSize; // 每个page里的每个block所占的字节大小（该值必须是256的整数倍）
};
