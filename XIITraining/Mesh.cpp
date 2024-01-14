#include "Mesh.h"
#include "Material.h"
#include "CommittedAllocator.h"
#include "MeshGenerator.h"

void Mesh::Init(const std::string& meshName)
{
	m_meshGeneratorName = meshName;
	CreateCBuffer();
}

void Mesh::CreateCBuffer()
{
	g_pCBufferAllocator->Alloc(sizeof(m_cbData), ResourceType_Upload, m_cbDataGPUVirtualAddr, m_cbDataCBufferPageIndex, m_cbDataByteOffset);

	// 创建CBV
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_cbDataGPUVirtualAddr; // 常量缓冲区的GPU虚拟地址
	cbvDesc.SizeInBytes = (UINT)((sizeof(m_cbData) + 255) & ~255);

	// 分配描述符
	D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle;
	UINT nouse[2];
	if (g_pDescriptorAllocator->Alloc(DescriptorType_CBV, 1, nouse[0], nouse[1], cbvCpuHandle))
		g_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);
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

	g_pCBufferAllocator->UpdateData(&m_cbData, sizeof(m_cbData), m_cbDataCBufferPageIndex, m_cbDataByteOffset);
}

void Mesh::Render()
{
	// cbPerObject
	g_pCommandList->SetGraphicsRootConstantBufferView(1, m_cbDataGPUVirtualAddr);

	const MeshViews& meshViews = MeshGenerator::GetInstance()->GetMeshViews(m_meshGeneratorName);
	g_pCommandList->IASetVertexBuffers(0, 1, &meshViews.vbv);
	g_pCommandList->IASetIndexBuffer(&meshViews.ibv);
	g_pCommandList->DrawIndexedInstanced(meshViews.indexCount, 1, 0, 0, 0);
}
