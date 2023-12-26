#include "CBufferAllocator.h"

void CBufferAllocator::CreateResourcePage(CBufferPage& newPage)
{
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(m_blockByteSize * CBUFFER_RESOURCE_ELEMENT_SIZE);
	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc, // ��Դ����
		D3D12_RESOURCE_STATE_GENERIC_READ, // ��ʼ����Դ״̬ΪREAD������CPUд�����ݣ�
		nullptr,
		IID_PPV_ARGS(&newPage.pData)
	);

	std::wstring debugName = L"CBuffer Resources Pool_" + std::to_wstring(m_pages.size());
	newPage.pData->SetName(debugName.c_str());
}
