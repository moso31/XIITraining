#include "PlacedAllocator.h"

bool PlacedAllocator::Alloc(const D3D12_RESOURCE_DESC& resourceDesc, ID3D12Resource** pOutResource)
{
	UINT layoutSize = resourceDesc.DepthOrArraySize * resourceDesc.MipLevels;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = new D3D12_PLACED_SUBRESOURCE_FOOTPRINT[layoutSize];
	UINT* numRow = new UINT[layoutSize];
	UINT64* numRowSizeInBytes = new UINT64[layoutSize];
	size_t totalBytes;
	m_pDevice->GetCopyableFootprints(&resourceDesc, 0, layoutSize, 0, layouts, numRow, numRowSizeInBytes, &totalBytes);

	size_t blockByteMask = m_blockByteSize - 1;
	UINT dataByteSize = (UINT)((totalBytes + blockByteMask) & ~blockByteMask);
	UINT blockSize = (UINT)totalBytes / m_blockByteSize;

	UINT oPageIdx, oFirstIdx;
	if (PlacedAllocatorBase::Alloc(blockSize, oPageIdx, oFirstIdx))
	{
		auto& pHeap = m_pages[oPageIdx].data;
		UINT heapByteOffset = m_blockByteSize * oFirstIdx;

		HRESULT hr = m_pDevice->CreatePlacedResource(pHeap, heapByteOffset, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(pOutResource));
		return SUCCEEDED(hr);
	}

	return false;
}

void PlacedAllocator::CreateNewPage(PlacedAllocatorBase::Page& newPage)
{
	D3D12_HEAP_DESC desc = {};
	desc.Alignment = 0;
	desc.Flags = D3D12_HEAP_FLAG_NONE;
	desc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	desc.SizeInBytes = m_blockByteSize * m_eachPageDataNum;

	m_pDevice->CreateHeap(&desc, IID_PPV_ARGS(&newPage.data));
}
