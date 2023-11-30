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

	// 创建一个默认堆和一个上传堆
	CD3DX12_HEAP_PROPERTIES defaultHeapProp(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES uploadHeapProp(D3D12_HEAP_TYPE_UPLOAD);

	// 创建一个顶点缓冲默认资源
	g_pDevice->CreateCommittedResource(
		&defaultHeapProp, // 指定堆的类型
		D3D12_HEAP_FLAG_NONE, // 堆的标志
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPNTC) * vertices.size()), // 资源描述
		D3D12_RESOURCE_STATE_GENERIC_READ, // 资源的初始状态，本样例中也可将其 平替成更高效的 D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER 
		nullptr, 
		IID_PPV_ARGS(&m_pVB) 
	);

	// 创建一个顶点缓冲上传资源
	g_pDevice->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexPNTC) * vertices.size()),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pVBUpload)
	);

	// 使用一个制式数据 subdata，记录 vertices 的顶点信息
	D3D12_SUBRESOURCE_DATA subData;
	subData.pData = vertices.data(); 
	subData.RowPitch = sizeof(VertexPNTC) * vertices.size();
	subData.SlicePitch = subData.RowPitch; 

	// 将数据 vertices，以 VBUpload 作为中介，最终传输到 VB 中。
	// 在这个过程中，VB 的资源状态发生了变化，所以需要设置两次 Barrier，第一次将其作为被Copy的目标；第二次恢复原先的设置。
	g_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pVB.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources(g_pCommandList.Get(), m_pVB.Get(), m_pVBUpload.Get(), 0, 0, 1, &subData);
	g_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_pVB.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	// 注意上面的流程实际上是对写入 cmdList 流程的记录。
	// 真正将数据传输到GPU中，还需要 cmdQueue 确实的执行了 cmdList 以后 才行。
	// 在此之前，VBUpload 不能被释放！

	// 然后对 IB 的创建过程进行和上面类似的操作。
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
