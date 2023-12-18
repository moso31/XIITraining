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

	// 在堆里找一段大小为 allocSize 的空间，并分配描述符
	void Alloc(DescriptorType type, UINT allocSize, UINT oFirstIndex);

	void CreateHeap(DescriptorType type);

	// 检查当前所有对应类型的堆中是否还有可用空间
	void CheckAllocable(DescriptorType type, UINT allocSize, UINT oFirstIndex);

private:
	ID3D12Device* m_pDevice;
	std::vector<DescriptorHeap> m_srvHeaps;
	std::vector<DescriptorHeap> m_cbvHeaps;
};
