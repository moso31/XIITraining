#include "CommittedAllocator.h"

bool CommittedAllocator::Alloc(UINT byteSize, ResourceType resourceType, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr, UINT& oPageIdx, UINT& oPageByteOffset)
{
	size_t blockByteMask = m_blockByteSize - 1;
	UINT dataByteSize = (UINT)((byteSize + blockByteMask) & ~blockByteMask);
	UINT blockSize = dataByteSize / m_blockByteSize; // ���alloc��Ҫʹ�ö��ٸ�Block

	auto predicate = [resourceType](Page& page) {
		return page.data.type == resourceType;
	};

	UINT oFirstIdx;
	if (CommittedAllocatorBase::Alloc(blockSize, oPageIdx, oFirstIdx, predicate))
	{
		auto& pResource = m_pages[oPageIdx].data.pResource;
		oPageByteOffset = m_blockByteSize * oFirstIdx;
		oGPUVirtualAddr = pResource->GetGPUVirtualAddress() + oPageByteOffset;
		return true;
	}

	return false;
}

void CommittedAllocator::UpdateCBData(void* data, UINT dataSize, UINT pageIdx, UINT pageByteOffset)
{
	auto& pResource = m_pages[pageIdx].data.pResource;

	// ֻ Map Ҫ�޸ĵ���һ�μ���
	D3D12_RANGE mapRange;
	mapRange.Begin = pageByteOffset;
	mapRange.End = pageByteOffset + dataSize;

	UINT8* pSrc;
	HRESULT hr = pResource->Map(0, &mapRange, reinterpret_cast<void**>(&pSrc));

	UINT8* pDest = pSrc + pageByteOffset;
	memcpy(pDest, data, dataSize);
}

void CommittedAllocator::CreateNewPage(CommittedAllocatorBase::Page& newPage)
{
	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = newPage.data.type == ResourceType_Default ? D3D12_HEAP_TYPE_DEFAULT : D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	// ��ʼ��Դ״̬��������ϴ��ѣ�����Ϊ�ɶ��������Ĭ�϶ѣ���ֱ����Ϊ����Ŀ�ꡣ
	auto initResourceState = newPage.data.type == ResourceType_Default ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_GENERIC_READ;

	auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(m_blockByteSize * m_eachPageDataNum);
	HRESULT hr = g_pDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc, // ��Դ����
		initResourceState,
		nullptr,
		IID_PPV_ARGS(&newPage.data.pResource)
	);

	std::wstring strType = newPage.data.type == ResourceType_Default ? L"Default" : L"Upload";
	std::wstring debugName = L"CBuffer Resources Pool_" + strType + L"_" + std::to_wstring(m_pages.size() - 1);
	newPage.data.pResource->SetName(debugName.c_str());
}
