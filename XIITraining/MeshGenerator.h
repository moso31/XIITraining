#pragma once
#include "header.h"
#include "Instance.h"
#include "../XAllocator/CommittedAllocator.h"
#include "../XAllocator/DescriptorAllocator.h"

struct MeshData
{
	D3D12_GPU_VIRTUAL_ADDRESS GPUVirtualAddress;
	UINT pageIndex;
	UINT pageByteOffset;
};

struct MeshViews
{
	D3D12_VERTEX_BUFFER_VIEW vbv;
	D3D12_INDEX_BUFFER_VIEW ibv;
	UINT vertexCount;
	UINT indexCount;
};

class MeshGenerator : public Instance<MeshGenerator>
{
public:
	MeshGenerator() {}
	~MeshGenerator() {}

	void Init();

	void CreateBox(const std::string& name, float size);
	//static void CreateSphere();
	//static void CreateCylinder();
	//static void Model(const std::string& name, const std::filesystem::path& filepath)

	const MeshViews& GetMeshViews(const std::string& name);

private:
	CommittedAllocator* m_vbAllocator;
	CommittedAllocator* m_ibAllocator;
	std::unordered_map<std::string, MeshViews> m_data;

	// 临时资源存放列表
	// 用于存放临时的上传堆资源，等待拷贝到默认堆
	// todo：找合适的时机清空此资源队列。
	std::vector<ComPtr<ID3D12Resource>> m_tempUploadList;
};
