#include "CBufferAllocator.h"

void CBufferAllocator::Create(size_t sizeInBytes, D3D12_HEAP_TYPE heapType)
{
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(D3DUtil::CalcBufferViewSize(sizeof(MeshTransformData)) * 2);
	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc, // ��Դ����
		D3D12_RESOURCE_STATE_GENERIC_READ, // ��ʼ����Դ״̬ΪREAD������CPUд�����ݣ�
		nullptr,
		IID_PPV_ARGS(&m_pObjectCBUpload)
	);
	m_pObjectCBUpload->SetName(L"Object CB Upload");

	auto cbvHandle = g_pDescriptorAllocator->Alloc(DescriptorType_CBV, 2);

	{
		// ����CBV�������� �������� �ĵ�3��4λ
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_pObjectCBUpload->GetGPUVirtualAddress(); // ������������GPU�����ַ
		cbvDesc.SizeInBytes = D3DUtil::CalcBufferViewSize(sizeof(MeshTransformData)); // �����������Ĵ�С
		g_pDevice->CreateConstantBufferView(&cbvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvHandle, 0, m_nCBSRUAVDescriptorSize));

		cbvDesc.BufferLocation += cbvDesc.SizeInBytes; // ƫ��һλ
		g_pDevice->CreateConstantBufferView(&cbvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvHandle, 1, m_nCBSRUAVDescriptorSize));
	}
}

void CBufferAllocator::Create(const std::string& name, UINT64 byteSize)
{
	auto& pResource = m_pResourcesMap[name];
	pResource.alignedByteSize = CalcBufferViewSize(byteSize);

	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(pResource.alignedByteSize * 16384);
	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc, // ��Դ����
		D3D12_RESOURCE_STATE_GENERIC_READ, // ��ʼ����Դ״̬ΪREAD������CPUд�����ݣ�
		nullptr,
		IID_PPV_ARGS(&pResource.pData)
	);

	std::wstring debugName(name.begin(), name.end());
	pResource.pData->SetName(debugName.c_str());
}
