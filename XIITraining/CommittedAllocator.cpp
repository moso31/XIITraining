#include "CommittedAllocator.h"

void CommittedAllocator::CreateNewPage(ResourceAllocator::Page& newPage)
{
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(m_blockByteSize * m_eachPageDataNum);
	HRESULT hr = g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc, // 资源描述
		D3D12_RESOURCE_STATE_GENERIC_READ, // 初始的资源状态为READ（允许CPU写入数据）
		nullptr,
		IID_PPV_ARGS(&newPage.data)
	);

	std::wstring debugName = L"CBuffer Resources Pool_" + std::to_wstring(m_pages.size() - 1);
	newPage.data->SetName(debugName.c_str());
}
