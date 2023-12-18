#include "header.h"

enum DescriptorType
{
	DescriptorType_CBV,
	DescriptorType_SRV,
	DescriptorType_UAV,
};

struct DescriptorHeap
{
	ID3D12DescriptorHeap* heap;
	DescriptorType heapType;
};

class DescriptorAllocator
{
public:
	DescriptorAllocator(ID3D12Device* pDevice);

	// �ڶ�����һ�δ�СΪ allocSize �Ŀռ䣬������������
	void Alloc(DescriptorType type, UINT allocSize, UINT oFirstIndex);

	void CreateHeap(DescriptorType type);

	// ��鵱ǰ���ж�Ӧ���͵Ķ����Ƿ��п��ÿռ�
	void CheckAllocable(DescriptorType type, UINT allocSize, UINT oFirstIndex);

private:
	ID3D12Device* m_pDevice;
	std::vector<DescriptorHeap> m_srvHeaps;
	std::vector<DescriptorHeap> m_cbvHeaps;
};
