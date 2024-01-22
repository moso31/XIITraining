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

	// ��ʱ��Դ����б�
	// ���ڴ����ʱ���ϴ�����Դ���ȴ�������Ĭ�϶�
	// todo���Һ��ʵ�ʱ����մ���Դ���С�
	std::vector<ComPtr<ID3D12Resource>> m_tempUploadList;
};
