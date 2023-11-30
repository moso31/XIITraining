#include "Mesh.h"
#include "ShaderStructures.h"

void Mesh::InitBox()
{
	std::vector<VertexPNTC> vertices = 
	{
		// +X face
		VertexPNTC(Vector3(1.0f, -1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f)),
		VertexPNTC(Vector3(1.0f, +1.0f, -1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f)),
		VertexPNTC(Vector3(1.0f, +1.0f, +1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f)),
		VertexPNTC(Vector3(1.0f, -1.0f, +1.0f), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f)),

		// -X face
		VertexPNTC(Vector3(-1.0f, -1.0f, +1.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, +1.0f, +1.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, +1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, -1.0f, -1.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f, 1.0f)),

		// +Y face
		VertexPNTC(Vector3(-1.0f, +1.0f, -1.0f), Vector3(0.0f, +1.0f, 0.0f), Vector2(1.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, +1.0f, +1.0f), Vector3(0.0f, +1.0f, 0.0f), Vector2(1.0f, 1.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f)),
		VertexPNTC(Vector3(+1.0f, +1.0f, +1.0f), Vector3(0.0f, +1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f)),
		VertexPNTC(Vector3(+1.0f, +1.0f, -1.0f), Vector3(0.0f, +1.0f, 0.0f), Vector2(0.0f, 0.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f)),

		// -Y face
		VertexPNTC(Vector3(-1.0f, -1.0f, +1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(1.0f, 0.0f), Vector4(1.0f, 1.0f, 0.0f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f), Vector4(1.0f, 1.0f, 0.0f, 1.0f)),
        VertexPNTC(Vector3(+1.0f, -1.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector4(1.0f, 1.0f, 0.0f, 1.0f)),
		VertexPNTC(Vector3(+1.0f, -1.0f, +1.0f), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.0f, 0.0f), Vector4(1.0f, 1.0f, 0.0f, 1.0f)),

        // +Z face
		VertexPNTC(Vector3(+1.0f, -1.0f, +1.0f), Vector3(0.0f, 0.0f, +1.0f), Vector2(1.0f, 0.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f)),
		VertexPNTC(Vector3(+1.0f, +1.0f, +1.0f), Vector3(0.0f, 0.0f, +1.0f), Vector2(1.0f, 1.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, +1.0f, +1.0f), Vector3(0.0f, 0.0f, +1.0f), Vector2(0.0f, 1.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, -1.0f, +1.0f), Vector3(0.0f, 0.0f, +1.0f), Vector2(0.0f, 0.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f)),

        // -Z face
		VertexPNTC(Vector3(+1.0f, +1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 0.0f), Vector4(0.0f, 1.0f, 1.0f, 1.0f)),
		VertexPNTC(Vector3(+1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 1.0f), Vector4(0.0f, 1.0f, 1.0f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, -1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 1.0f), Vector4(0.0f, 1.0f, 1.0f, 1.0f)),
		VertexPNTC(Vector3(-1.0f, +1.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 0.0f), Vector4(0.0f, 1.0f, 1.0f, 1.0f))
	};

	// ����һ��Ĭ�϶Ѻ�һ���ϴ���
	CD3DX12_HEAP_PROPERTIES defaultHeapProp(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	// ����һ�����㻺��Ĭ����Դ
	g_pDevice->CreateCommittedResource(
		&defaultHeapProp, // ָ���ѵ�����
		D3D12_HEAP_FLAG_NONE, // �ѵı�־
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPNTC) * vertices.size()), // ��Դ����
		D3D12_RESOURCE_STATE_GENERIC_READ, // ��Դ�ĳ�ʼ״̬����������Ҳ�ɽ��� ƽ��ɸ���Ч�� D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER 
		nullptr, 
		IID_PPV_ARGS(&m_pVB) 
	);

	// ����һ�����㻺���ϴ���Դ
	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPNTC) * vertices.size()),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pVBUpload)
	);

	// ʹ��һ����ʽ���� subdata����¼ vertices �Ķ�����Ϣ
	D3D12_SUBRESOURCE_DATA subData;
	subData.pData = vertices.data(); 
	subData.RowPitch = sizeof(VertexPNTC) * vertices.size();
	subData.SlicePitch = subData.RowPitch; 

	// ������ vertices���� VBUpload ��Ϊ�н飬���մ��䵽 VB �С�
	// ����������У�VB ����Դ״̬�����˱仯��������Ҫ�������� Barrier����һ�ν�����Ϊ��Copy��Ŀ�ꣻ�ڶ��λָ�ԭ�ȵ����á�
	g_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pVB.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(g_pCommandList.Get(), m_pVB.Get(), m_pVBUpload.Get(), 0, 0, 1, &subData);
	g_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pVB.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	// ע�����������ʵ�����Ƕ�д�� cmdList ���̵ļ�¼��
	// ���������ݴ��䵽GPU�У�����Ҫ cmdQueue ȷʵ��ִ���� cmdList �Ժ� ���С�
	// �ڴ�֮ǰ��VBUpload ���ܱ��ͷţ�

	// Ȼ��� IB �Ĵ������̽��к��������ƵĲ�����
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
}
