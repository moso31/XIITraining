#include "MeshGenerator.h"

void MeshGenerator::Init()
{
	m_allocator = new CommittedAllocator(g_pDevice.Get(), g_pDescriptorAllocator);
}

void MeshGenerator::CreateBox(int size)
{
	std::vector<VertexPNTC> vertices = 
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

	//m_allocator->AllocCBV();
}
