#include "CBufferAllocator.h"

void CBufferAllocator::Init(UINT blockByteSize)
{
	m_pResource.blockByteSize = 256;
	m_pResource.byteOffset = 0;

	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(m_pResource.blockByteSize * CBUFFER_RESOURCE_ELEMENT_SIZE);
	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc, // 资源描述
		D3D12_RESOURCE_STATE_GENERIC_READ, // 初始的资源状态为READ（允许CPU写入数据）
		nullptr,
		IID_PPV_ARGS(&m_pResource.pData)
	);

	m_pResource.pData->SetName(L"CBuffer Resources Pool");
}
