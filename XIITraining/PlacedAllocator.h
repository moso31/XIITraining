#pragma once
#include "Texture.h"
#include "DescriptorAllocator.h"

#define PlacedAllocatorBase XAllocator<ID3D12Heap*>

class PlacedAllocator : PlacedAllocatorBase
{
public:
	PlacedAllocator(ID3D12Device* pDevice, DescriptorAllocator* pDescriptorAllocator, UINT blockByteSize = 256) : 
		PlacedAllocatorBase(1000000, 10), m_pDevice(pDevice), m_pDescriptorAllocator(pDescriptorAllocator), m_blockByteSize(blockByteSize) {}
	~PlacedAllocator() {}

	void Alloc(ID3D12Resource* pResource);

	void CreateNewPage(PlacedAllocatorBase::Page& newPage) override;

private:
	ID3D12Device* m_pDevice;
	DescriptorAllocator* m_pDescriptorAllocator;

	UINT m_blockByteSize; // ÿ��page���ÿ��block��ռ���ֽڴ�С����ֵ������256����������
};
