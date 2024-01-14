#pragma once
#include "header.h"
#include "Instance.h"
#include "CommittedAllocator.h"
#include "DescriptorAllocator.h"

typedef uint64_t MeshHash;

class MeshGenerator : public Instance<MeshGenerator>
{
public:
	MeshGenerator() {}
	~MeshGenerator() {}

	void Init();

	void CreateBox(int size);
	//static void CreateSphere();
	//static void CreateCylinder();
	//static void Model(const std::filesystem::path& filepath)

private:
	CommittedAllocator* m_allocator;
	//std::unordered_map<MeshHash, >;
};
