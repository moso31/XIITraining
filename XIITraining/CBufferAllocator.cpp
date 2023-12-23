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
		&cbDesc, // ��Դ����
		D3D12_RESOURCE_STATE_GENERIC_READ, // ��ʼ����Դ״̬ΪREAD������CPUд�����ݣ�
		nullptr,
		IID_PPV_ARGS(&m_pResource.pData)
	);

	m_pResource.pData->SetName(L"CBuffer Resources Pool");
}
