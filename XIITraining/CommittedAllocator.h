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
	bool Alloc(UINT byteSize, ResourceType resourceType, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr, UINT& oPageIdx, UINT& oPageByteOffset);
	 
	void UpdateCBData(void* data, UINT dataSize, UINT pageIdx, UINT pageByteOffset);

	void CreateNewPage(CommittedAllocatorBase::Page& newPage) override;

private:
	ID3D12Device* m_pDevice;
	DescriptorAllocator* m_pDescriptorAllocator;

	UINT m_blockByteSize; // ÿ��page���ÿ��block��ռ���ֽڴ�С����ֵ������256����������
};
