#include "Mesh.h"
#include "Material.h"
#include "CommittedAllocator.h"

void Mesh::InitBox()
{
	m_vertices = 
	{
		// +X face
		VertexPNTC(Vector3(1.0f, -1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector4(0.5f, 0.75f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(1.0f, +1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f), Vector4(0.5f, 0.75f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(1.0f, +1.0f, +1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f), Vector4(0.5f, 0.75f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(1.0f, -1.0f, +1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f), Vector4(0.5f, 0.75f, 0.75f, 1.0f)),

		// -X face
		VertexPNTC(Vector3(-1.0f, -1.0f, +1.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector4(0.75f, 0.5f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, +1.0f, +1.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f), Vector4(0.75f, 0.5f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, +1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f), Vector4(0.75f, 0.5f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f), Vector4(0.75f, 0.5f, 0.75f, 1.0f)),

		// +Y face
		VertexPNTC(Vector3(-1.0f, +1.0f, -1.0f), Vector3(0.0f, +1.0f, 0.0f), Vector2(1.0f, 0.0f), Vector4(0.75f, 0.75f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, +1.0f, +1.0f), Vector3(0.0f, +1.0f, 0.0f), Vector2(1.0f, 1.0f), Vector4(0.75f, 0.75f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(+1.0f, +1.0f, +1.0f), Vector3(0.0f, +1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector4(0.75f, 0.75f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(+1.0f, +1.0f, -1.0f), Vector3(0.0f, +1.0f, 0.0f), Vector2(0.0f, 0.0f), Vector4(0.75f, 0.75f, 0.5f, 1.0f)),

		// -Y face
		VertexPNTC(Vector3(-1.0f, -1.0f, +1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(1.0f, 0.0f), Vector4(0.5f, 0.5f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f), Vector4(0.5f, 0.5f, 0.75f, 1.0f)),
        VertexPNTC(Vector3(+1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector4(0.5f, 0.5f, 0.75f, 1.0f)),
		VertexPNTC(Vector3(+1.0f, -1.0f, +1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.0f, 0.0f), Vector4(0.5f, 0.5f, 0.75f, 1.0f)),

        // +Z face
		VertexPNTC(Vector3(+1.0f, -1.0f, +1.0f), Vector3(0.0f, 0.0f, +1.0f), Vector2(1.0f, 0.0f), Vector4(0.5f, 0.75f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(+1.0f, +1.0f, +1.0f), Vector3(0.0f, 0.0f, +1.0f), Vector2(1.0f, 1.0f), Vector4(0.5f, 0.75f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, +1.0f, +1.0f), Vector3(0.0f, 0.0f, +1.0f), Vector2(0.0f, 1.0f), Vector4(0.5f, 0.75f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, -1.0f, +1.0f), Vector3(0.0f, 0.0f, +1.0f), Vector2(0.0f, 0.0f), Vector4(0.5f, 0.75f, 0.5f, 1.0f)),

        // -Z face
		VertexPNTC(Vector3(+1.0f, +1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector4(0.75f, 0.5f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(+1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 1.0f), Vector4(0.75f, 0.5f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 1.0f), Vector4(0.75f, 0.5f, 0.5f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, +1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 0.0f), Vector4(0.75f, 0.5f, 0.5f, 1.0f))
	};

	m_indices =
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

	CreateVB();
	CreateIB();

	CreateCBuffer();
}

void Mesh::CreateVB()
{
	// 创建一个默认堆和一个上传堆
	CD3DX12_HEAP_PROPERTIES defaultHeapProp(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	CD3DX12_RESOURCE_DESC vbDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPNTC) * m_vertices.size());

	// 创建一个顶点缓冲默认资源
	g_pDevice->CreateCommittedResource(
		&defaultHeapProp, // 指定堆的类型
		D3D12_HEAP_FLAG_NONE, // 堆的标志
		&vbDesc, // 资源描述
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_pVB)
	);
	m_pVB->SetName(L"Mesh VB");

	// 创建一个顶点缓冲上传资源
	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&vbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // 初始的资源状态为COPY_DEST（复制目标，因为接下来必然有一个 从上传堆将数据复制过来的过程）
		nullptr,
		IID_PPV_ARGS(&m_pVBUpload)
	);
	m_pVBUpload->SetName(L"Mesh VB Upload");

	// 使用一个制式数据 subdata，记录 m_vertices 的顶点信息
	D3D12_SUBRESOURCE_DATA subData;
	subData.pData = m_vertices.data();
	subData.RowPitch = sizeof(VertexPNTC) * m_vertices.size();
	subData.SlicePitch = subData.RowPitch;

	// 将数据 m_vertices，以 VBUpload 作为中介，最终传输到 VB 中。
	void* mappedData;
	m_pVBUpload->Map(0, nullptr, &mappedData);
	memcpy(mappedData, m_vertices.data(), sizeof(VertexPNTC) * m_vertices.size()); 
	m_pVBUpload->Unmap(0, nullptr);
	g_pCommandList->CopyBufferRegion(m_pVB.Get(), 0, m_pVBUpload.Get(), 0, subData.RowPitch);

	// 当 GPU 将上述流程执行完毕，VB 的资源状态将发生了变化，所以需要改变 m_pVB 标记的资源状态，改为正式的VertexBuffer类型：
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pVB.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	g_pCommandList->ResourceBarrier(1, &barrier);

	// 注意：上面的流程只是对写入 cmdList 流程的记录。实际的GPU逻辑此时并未执行！
	// 真正将数据传输到GPU中，还需要 cmdQueue 确实的执行了 cmdList 以后 才行。
	// 在GPU真正将这段逻辑执行完之前，这些 Resource 都不能被释放！
}

void Mesh::CreateIB()
{
	// 注释略，同UpdateVB。
	CD3DX12_HEAP_PROPERTIES defaultHeapProp(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	auto ibDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT) * m_indices.size());

	g_pDevice->CreateCommittedResource(
		&defaultHeapProp, // 指定堆的类型
		D3D12_HEAP_FLAG_NONE, // 堆的标志
		&ibDesc, // 资源描述
		D3D12_RESOURCE_STATE_COPY_DEST, // 初始的资源状态为COPY_DEST（复制目标，因为接下来必然有一个 从上传堆将数据复制过来的过程）
		nullptr,
		IID_PPV_ARGS(&m_pIB)
	);
	m_pIB->SetName(L"Mesh IB");

	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&ibDesc, // 资源描述
		D3D12_RESOURCE_STATE_GENERIC_READ, // 初始的资源状态为READ（允许CPU写入数据）
		nullptr,
		IID_PPV_ARGS(&m_pIBUpload)
	);
	m_pIBUpload->SetName(L"Mesh IB Upload");

	D3D12_SUBRESOURCE_DATA subData;
	subData.pData = m_indices.data();
	subData.RowPitch = sizeof(UINT) * m_indices.size();
	subData.SlicePitch = subData.RowPitch;

	void* mappedData;
	m_pIBUpload->Map(0, nullptr, &mappedData);
	memcpy(mappedData, m_indices.data(), sizeof(UINT) * m_indices.size());
	m_pIBUpload->Unmap(0, nullptr);
	g_pCommandList->CopyBufferRegion(m_pIB.Get(), 0, m_pIBUpload.Get(), 0, subData.RowPitch);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pIB.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	g_pCommandList->ResourceBarrier(1, &barrier);
}

void Mesh::CreateCBuffer()
{
	g_pCBufferAllocator->AllocCBV(m_cbData, m_cbDataGPUVirtualAddr, m_cbDataCBufferPageIndex, m_cbDataByteOffset);
}

void Mesh::SetScale(float x, float y, float z)
{
	m_scale = { x, y, z };
}

void Mesh::SetMaterial(Material* pMaterial)
{
	m_pMaterial = pMaterial;
	m_pMaterial->AddRefMesh(this);
}

void Mesh::Update()
{
	static float r = 0.0f;
	r += 0.0025f;

	Matrix mx = Matrix::CreateScale(m_scale);
	if (m_rotate) mx = mx * Matrix::CreateRotationX(-r) * Matrix::CreateRotationY(r);

	m_cbData.worldMatrix = mx;

	g_pCBufferAllocator->UpdateCBData(m_cbData, m_cbDataCBufferPageIndex, m_cbDataByteOffset);
}

void Mesh::Render()
{
	// cbPerObject
	g_pCommandList->SetGraphicsRootConstantBufferView(1, m_cbDataGPUVirtualAddr);

	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = m_pVB->GetGPUVirtualAddress();
	vbv.StrideInBytes = sizeof(VertexPNTC); // 每个顶点的大小
	vbv.SizeInBytes = vbv.StrideInBytes * (UINT)m_vertices.size(); // 整个顶点缓冲区的大小
	g_pCommandList->IASetVertexBuffers(0, 1, &vbv);

	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = m_pIB->GetGPUVirtualAddress();
	ibv.SizeInBytes = sizeof(UINT) * (UINT)m_indices.size();
	ibv.Format = DXGI_FORMAT_R32_UINT;
	g_pCommandList->IASetIndexBuffer(&ibv);

	g_pCommandList->DrawIndexedInstanced((UINT)m_indices.size(), 1, 0, 0, 0);
}
