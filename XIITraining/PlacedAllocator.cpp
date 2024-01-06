#include "PlacedAllocator.h"

void PlacedAllocator::Alloc(ID3D12Resource* pUploadResource)
{
	// ���� Texture ���Լ����ÿ�������Լ��� Upload���ϴ���CPU�ڴ����ݣ�
	// ���� Alloc ��ְ����ǣ�����������Ĭ�϶ѣ��� ID3D12Heap �С�

	const auto& desc = pUploadResource->GetDesc();
	UINT layoutSize = desc.DepthOrArraySize * desc.MipLevels;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = new D3D12_PLACED_SUBRESOURCE_FOOTPRINT[layoutSize];
	UINT* numRow = new UINT[layoutSize];
	UINT64* numRowSizeInBytes = new UINT64[layoutSize];
	size_t totalBytes;
	g_pDevice->GetCopyableFootprints(&desc, 0, layoutSize, 0, layouts, numRow, numRowSizeInBytes, &totalBytes);

	size_t blockByteMask = m_blockByteSize - 1;
	UINT dataByteSize = (UINT)((totalBytes + blockByteMask) & ~blockByteMask);
	UINT blockSize = totalBytes / m_blockByteSize;

	UINT oPageIdx, oFirstIdx;
	if (PlacedAllocatorBase::Alloc(blockSize, oPageIdx, oFirstIdx))
	{
		auto& pHeap = m_pages[oPageIdx].data;
		UINT heapByteOffset = m_blockByteSize * oFirstIdx;

		ComPtr<ID3D12Resource> pTexture;
		m_pDevice->CreatePlacedResource(pHeap, heapByteOffset, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&pTexture));
	}
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
