#pragma once
#include "header.h"

struct CBDataMesh
{
	Matrix worldMatrix;
};

class Material;
class Mesh
{
public:
	void Init(const std::string& meshName);

	void Update(const UINT swapChainBufferIndex);
	void Render(const UINT swapChainBufferIndex);

	void SetScale(float x, float y, float z);
	void SetRotate(bool val) { m_rotate = val; }

	void SetMaterial(Material* pMaterial);

private:
	void CreateCBuffer();

private:
	bool m_rotate;
	Vector3 m_scale;

	MultiFrame<XAllocatorData<CBDataMesh>> m_cbData;

	// 本Demo中 一个网格只有一个材质
	Material* m_pMaterial;

	std::string m_meshGeneratorName;
};

