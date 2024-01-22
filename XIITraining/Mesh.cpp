#include "Mesh.h"
#include "Material.h"
#include "..\XAllocator\CommittedAllocator.h"
#include "MeshGenerator.h"

void Mesh::Init(const std::string& meshName)
{
	m_meshGeneratorName = meshName;
	CreateCBuffer();
}

void Mesh::CreateCBuffer()
{
	for (int i = 0; i < FRAME_BUFFER_NUM; i++)
	{
		g_pCBufferAllocator->Alloc(m_cbData[i].DataByteSize(), ResourceType_Upload, m_cbData[i].GPUVirtualAddr, m_cbData[i].pageIndex, m_cbData[i].pageByteOffset);

		// 创建CBV
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_cbData[i].GPUVirtualAddr; // 常量缓冲区的GPU虚拟地址
		cbvDesc.SizeInBytes = (UINT)((m_cbData[i].DataByteSize() + 255) & ~255);

		// 分配描述符
		D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle;
		UINT nouse[2];
		if (g_pDescriptorAllocator->Alloc(DescriptorType_CBV, 1, nouse[0], nouse[1], cbvCpuHandle))
			g_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);
	}
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

void Mesh::Update(const UINT swapChainBufferIndex)
{
	static float r = 0.0f;
	r += 0.0025f;

	Matrix mx = Matrix::CreateScale(m_scale);
	if (m_rotate) mx = mx * Matrix::CreateRotationX(-r) * Matrix::CreateRotationY(r);

	auto& currFrameData = m_cbData[swapChainBufferIndex];
	currFrameData.data.worldMatrix = mx;

	g_pCBufferAllocator->UpdateData(&currFrameData.data, currFrameData.DataByteSize(), currFrameData.pageIndex, currFrameData.pageByteOffset);
}

void Mesh::Render(const UINT swapChainBufferIndex)
{
	// cbPerObject
	g_pCommandList->SetGraphicsRootConstantBufferView(1, m_cbData[swapChainBufferIndex].GPUVirtualAddr);

	const MeshViews& meshViews = MeshGenerator::GetInstance()->GetMeshViews(m_meshGeneratorName);
	g_pCommandList->IASetVertexBuffers(0, 1, &meshViews.vbv);
	g_pCommandList->IASetIndexBuffer(&meshViews.ibv);
	g_pCommandList->DrawIndexedInstanced(meshViews.indexCount, 1, 0, 0, 0);
}
