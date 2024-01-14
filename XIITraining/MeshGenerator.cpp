#include "MeshGenerator.h"

void MeshGenerator::Init()
{
	m_vbAllocator = new CommittedAllocator(g_pDevice.Get());
	m_ibAllocator = new CommittedAllocator(g_pDevice.Get());
}

void MeshGenerator::CreateBox(const std::string& name, float size)
{
	std::vector<VertexPNTC> vertices = 
	{
		// +X face
		VertexPNTC(Vector3(size, -size, -size), Vector3(size, 0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector4(0.5f, 0.75f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(size, +size, -size), Vector3(size, 0.0f, 0.0f), Vector2(1.0f, 1.0f), Vector4(0.5f, 0.75f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(size, +size, +size), Vector3(size, 0.0f, 0.0f), Vector2(0.0f, 1.0f), Vector4(0.5f, 0.75f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(size, -size, +size), Vector3(size, 0.0f, 0.0f), Vector2(0.0f, 0.0f), Vector4(0.5f, 0.75f, 0.75f, 1.0f)),

		// -X face
		VertexPNTC(Vector3(-size, -size, +size), Vector3(-size, 0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector4(0.75f, 0.5f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(-size, +size, +size), Vector3(-size, 0.0f, 0.0f), Vector2(1.0f, 1.0f), Vector4(0.75f, 0.5f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(-size, +size, -size), Vector3(-size, 0.0f, 0.0f), Vector2(0.0f, 1.0f), Vector4(0.75f, 0.5f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(-size, -size, -size), Vector3(-size, 0.0f, 0.0f), Vector2(0.0f, 0.0f), Vector4(0.75f, 0.5f, 0.75f, 1.0f)),

		// +Y face
		VertexPNTC(Vector3(-size, +size, -size), Vector3(0.0f, +size, 0.0f), Vector2(1.0f, 0.0f), Vector4(0.75f, 0.75f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(-size, +size, +size), Vector3(0.0f, +size, 0.0f), Vector2(1.0f, 1.0f), Vector4(0.75f, 0.75f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(+size, +size, +size), Vector3(0.0f, +size, 0.0f), Vector2(0.0f, 1.0f), Vector4(0.75f, 0.75f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(+size, +size, -size), Vector3(0.0f, +size, 0.0f), Vector2(0.0f, 0.0f), Vector4(0.75f, 0.75f, 0.5f, 1.0f)),

		// -Y face
		VertexPNTC(Vector3(-size, -size, +size), Vector3(0.0f, -size, 0.0f), Vector2(1.0f, 0.0f), Vector4(0.5f, 0.5f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(-size, -size, -size), Vector3(0.0f, -size, 0.0f), Vector2(1.0f, 1.0f), Vector4(0.5f, 0.5f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(+size, -size, -size), Vector3(0.0f, -size, 0.0f), Vector2(0.0f, 1.0f), Vector4(0.5f, 0.5f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(+size, -size, +size), Vector3(0.0f, -size, 0.0f), Vector2(0.0f, 0.0f), Vector4(0.5f, 0.5f, 0.75f, 1.0f)),

		// +Z face
		VertexPNTC(Vector3(+size, -size, +size), Vector3(0.0f, 0.0f, +size), Vector2(1.0f, 0.0f), Vector4(0.5f, 0.75f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(+size, +size, +size), Vector3(0.0f, 0.0f, +size), Vector2(1.0f, 1.0f), Vector4(0.5f, 0.75f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(-size, +size, +size), Vector3(0.0f, 0.0f, +size), Vector2(0.0f, 1.0f), Vector4(0.5f, 0.75f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(-size, -size, +size), Vector3(0.0f, 0.0f, +size), Vector2(0.0f, 0.0f), Vector4(0.5f, 0.75f, 0.5f, 1.0f)),

		// -Z face
		VertexPNTC(Vector3(+size, +size, -size), Vector3(0.0f, 0.0f, -size), Vector2(1.0f, 0.0f), Vector4(0.75f, 0.5f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(+size, -size, -size), Vector3(0.0f, 0.0f, -size), Vector2(1.0f, 1.0f), Vector4(0.75f, 0.5f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(-size, -size, -size), Vector3(0.0f, 0.0f, -size), Vector2(0.0f, 1.0f), Vector4(0.75f, 0.5f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(-size, +size, -size), Vector3(0.0f, 0.0f, -size), Vector2(0.0f, 0.0f), Vector4(0.75f, 0.5f, 0.5f, 1.0f))
	};

	std::vector<UINT> indices =
	{
		// -X face
		0, 1, 2,
		0, 2, 3,
		// +X face
		4, 5, 6,
		4, 6, 7,
		// +Y face
		8, 9, 10,
		8, 10, 11,
		// -Y face
		12, 13, 14,
		12, 14, 15,
		// +Z face
		16, 17, 18,
		16, 18, 19,
		// -Z face
		20, 21, 22,
		20, 22, 23
	};

	UINT vbSize = (UINT)vertices.size() * sizeof(VertexPNTC);
	UINT ibSize = (UINT)indices.size() * sizeof(UINT);

	// 1. 在默认堆中分配这个Mesh的顶点和索引内存
	MeshData vbData, ibData;
	bool vbAlloc = m_vbAllocator->Alloc(vbSize, ResourceType_Default, vbData.GPUVirtualAddress, vbData.pageIndex, vbData.pageByteOffset);
	bool ibAlloc = m_ibAllocator->Alloc(ibSize, ResourceType_Default, ibData.GPUVirtualAddress, ibData.pageIndex, ibData.pageByteOffset);

	if (vbAlloc && ibAlloc)
	{
		if (m_data.find(name) != m_data.end()) 
			throw std::exception("Mesh name already exists.");

		// 2. 将顶点和索引数据拷贝到默认堆中，但这需要先准备上传堆，然后从上传堆拷贝到默认堆。
		// 规定了只存储静态Mesh，所以不用考虑上传堆备份的问题。
		CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC vbDesc = CD3DX12_RESOURCE_DESC::Buffer(vbSize);
		CD3DX12_RESOURCE_DESC ibDesc = CD3DX12_RESOURCE_DESC::Buffer(ibSize);

		// 2.1 准备上传堆
		ComPtr<ID3D12Resource> uploadVB;
		ComPtr<ID3D12Resource> uploadIB;
		HRESULT hr = g_pDevice->CreateCommittedResource(
			&uploadHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&vbDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadVB)
		);
		uploadVB->SetName(L"Temp uploadVB");

		hr = g_pDevice->CreateCommittedResource(
			&uploadHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&ibDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadIB)
		);
		uploadIB->SetName(L"Temp uploadIB");

		m_tempUploadList.emplace_back(uploadVB);
		m_tempUploadList.emplace_back(uploadIB);

		// 2.2 将顶点和索引数据拷贝到上传堆
		UINT vbDataSize = (UINT)(sizeof(VertexPNTC) * vertices.size());
		UINT ibDataSize = (UINT)(sizeof(UINT) * indices.size());

		void* mappedData;
		uploadVB->Map(0, nullptr, &mappedData);
		memcpy(mappedData, vertices.data(), vbDataSize);
		uploadVB->Unmap(0, nullptr);

		uploadIB->Map(0, nullptr, &mappedData);
		memcpy(mappedData, indices.data(), ibDataSize);
		uploadIB->Unmap(0, nullptr);

		// 2.3 从上传堆拷贝到默认堆
		m_vbAllocator->SetResourceState(vbData.pageIndex, D3D12_RESOURCE_STATE_COPY_DEST);
		m_vbAllocator->UpdateData(uploadVB.Get(), vbDataSize, vbData.pageIndex, vbData.pageByteOffset);
		m_vbAllocator->SetResourceState(vbData.pageIndex, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		m_ibAllocator->SetResourceState(ibData.pageIndex, D3D12_RESOURCE_STATE_COPY_DEST);
		m_ibAllocator->UpdateData(uploadIB.Get(), ibDataSize, ibData.pageIndex, ibData.pageByteOffset);
		m_ibAllocator->SetResourceState(ibData.pageIndex, D3D12_RESOURCE_STATE_INDEX_BUFFER);

		// 3. 将Mesh数据存储到m_data中
		MeshViews views;
		views.vbv.BufferLocation = vbData.GPUVirtualAddress;
		views.vbv.SizeInBytes = vbDataSize;
		views.vbv.StrideInBytes = sizeof(VertexPNTC);
		views.ibv.BufferLocation = ibData.GPUVirtualAddress;
		views.ibv.SizeInBytes = ibDataSize;
		views.ibv.Format = DXGI_FORMAT_R32_UINT;
		views.indexCount = (UINT)indices.size();
		views.vertexCount = (UINT)vertices.size();
		m_data[name] = std::move(views);
	}
}

const MeshViews& MeshGenerator::GetMeshViews(const std::string& name)
{
	if (m_data.find(name) == m_data.end())
		std::exception("Mesh name not found.");

	return m_data[name];
}
