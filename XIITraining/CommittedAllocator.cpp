#include "CommittedAllocator.h"

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
