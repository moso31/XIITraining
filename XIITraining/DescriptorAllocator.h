#include "header.h"

enum DescriptorType
{
	DescriptorType_CBV,
	DescriptorType_SRV,
	DescriptorType_UAV,
};

struct DescriptorHeap
{
	ID3D12DescriptorHeap* data;
	DescriptorType type;
};

class DescriptorAllocator
{
public:
	DescriptorAllocator(ID3D12Device* pDevice);

	// �ڶ�����һ�δ�СΪ allocSize �Ŀռ䣬������������
	void Alloc(DescriptorType type, UINT allocSize, UINT oFirstIndex);

	void CreateHeap(DescriptorType type);

	// ��鵱ǰ���ж�Ӧ���͵Ķ����Ƿ��п��ÿռ�
	bool CheckAllocable(DescriptorType type, UINT allocSize, UINT oHeapIdx, UINT oViewIdx);

private:
	ID3D12Device* m_pDevice;
	std::vector<DescriptorHeap> m_heaps;
};
