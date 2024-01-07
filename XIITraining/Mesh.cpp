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
	// ����һ��Ĭ�϶Ѻ�һ���ϴ���
	CD3DX12_HEAP_PROPERTIES defaultHeapProp(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	CD3DX12_RESOURCE_DESC vbDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPNTC) * m_vertices.size());

	// ����һ�����㻺��Ĭ����Դ
	g_pDevice->CreateCommittedResource(
		&defaultHeapProp, // ָ���ѵ�����
		D3D12_HEAP_FLAG_NONE, // �ѵı�־
		&vbDesc, // ��Դ����
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_pVB)
	);
	m_pVB->SetName(L"Mesh VB");

	// ����һ�����㻺���ϴ���Դ
	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&vbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, // ��ʼ����Դ״̬ΪCOPY_DEST������Ŀ�꣬��Ϊ��������Ȼ��һ�� ���ϴ��ѽ����ݸ��ƹ����Ĺ��̣�
		nullptr,
		IID_PPV_ARGS(&m_pVBUpload)
	);
	m_pVBUpload->SetName(L"Mesh VB Upload");

	// ʹ��һ����ʽ���� subdata����¼ m_vertices �Ķ�����Ϣ
	D3D12_SUBRESOURCE_DATA subData;
	subData.pData = m_vertices.data();
	subData.RowPitch = sizeof(VertexPNTC) * m_vertices.size();
	subData.SlicePitch = subData.RowPitch;

	// ������ m_vertices���� VBUpload ��Ϊ�н飬���մ��䵽 VB �С�
	void* mappedData;
	m_pVBUpload->Map(0, nullptr, &mappedData);
	memcpy(mappedData, m_vertices.data(), sizeof(VertexPNTC) * m_vertices.size()); 
	m_pVBUpload->Unmap(0, nullptr);
	g_pCommandList->CopyBufferRegion(m_pVB.Get(), 0, m_pVBUpload.Get(), 0, subData.RowPitch);

	// �� GPU ����������ִ����ϣ�VB ����Դ״̬�������˱仯��������Ҫ�ı� m_pVB ��ǵ���Դ״̬����Ϊ��ʽ��VertexBuffer���ͣ�
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pVB.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	g_pCommandList->ResourceBarrier(1, &barrier);

	// ע�⣺���������ֻ�Ƕ�д�� cmdList ���̵ļ�¼��ʵ�ʵ�GPU�߼���ʱ��δִ�У�
	// ���������ݴ��䵽GPU�У�����Ҫ cmdQueue ȷʵ��ִ���� cmdList �Ժ� ���С�
	// ��GPU����������߼�ִ����֮ǰ����Щ Resource �����ܱ��ͷţ�
}

void Mesh::CreateIB()
{
	// ע���ԣ�ͬUpdateVB��
	CD3DX12_HEAP_PROPERTIES defaultHeapProp(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	auto ibDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT) * m_indices.size());

	g_pDevice->CreateCommittedResource(
		&defaultHeapProp, // ָ���ѵ�����
		D3D12_HEAP_FLAG_NONE, // �ѵı�־
		&ibDesc, // ��Դ����
		D3D12_RESOURCE_STATE_COPY_DEST, // ��ʼ����Դ״̬ΪCOPY_DEST������Ŀ�꣬��Ϊ��������Ȼ��һ�� ���ϴ��ѽ����ݸ��ƹ����Ĺ��̣�
		nullptr,
		IID_PPV_ARGS(&m_pIB)
	);
	m_pIB->SetName(L"Mesh IB");

	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&ibDesc, // ��Դ����
		D3D12_RESOURCE_STATE_GENERIC_READ, // ��ʼ����Դ״̬ΪREAD������CPUд�����ݣ�
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
	vbv.StrideInBytes = sizeof(VertexPNTC); // ÿ������Ĵ�С
	vbv.SizeInBytes = vbv.StrideInBytes * (UINT)m_vertices.size(); // �������㻺�����Ĵ�С
	g_pCommandList->IASetVertexBuffers(0, 1, &vbv);

	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = m_pIB->GetGPUVirtualAddress();
	ibv.SizeInBytes = sizeof(UINT) * (UINT)m_indices.size();
	ibv.Format = DXGI_FORMAT_R32_UINT;
	g_pCommandList->IASetIndexBuffer(&ibv);

	g_pCommandList->DrawIndexedInstanced((UINT)m_indices.size(), 1, 0, 0, 0);
}
