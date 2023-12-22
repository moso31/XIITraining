#include "CBufferAllocator.h"

void CBufferAllocator::Create(size_t sizeInBytes, D3D12_HEAP_TYPE heapType)
{
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(D3DUtil::CalcBufferViewSize(sizeof(MeshTransformData)) * 2);
	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc, // 资源描述
		D3D12_RESOURCE_STATE_GENERIC_READ, // 初始的资源状态为READ（允许CPU写入数据）
		nullptr,
		IID_PPV_ARGS(&m_pObjectCBUpload)
	);
	m_pObjectCBUpload->SetName(L"Object CB Upload");

	auto cbvHandle = g_pDescriptorAllocator->Alloc(DescriptorType_CBV, 2);

	{
		// 创建CBV，并放在 描述符堆 的第3、4位
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_pObjectCBUpload->GetGPUVirtualAddress(); // 常量缓冲区的GPU虚拟地址
		cbvDesc.SizeInBytes = D3DUtil::CalcBufferViewSize(sizeof(MeshTransformData)); // 常量缓冲区的大小
		g_pDevice->CreateConstantBufferView(&cbvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(cbvHandle, 0, m_nCBSRUAVDescriptorSize));

		cbvDesc.BufferLocation += cbvDesc.SizeInBytes; // 偏移一位
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
		&cbDesc, // 资源描述
		D3D12_RESOURCE_STATE_GENERIC_READ, // 初始的资源状态为READ（允许CPU写入数据）
		nullptr,
		IID_PPV_ARGS(&pResource.pData)
	);

	std::wstring debugName(name.begin(), name.end());
	pResource.pData->SetName(debugName.c_str());
}
