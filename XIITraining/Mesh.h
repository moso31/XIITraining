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

	void Update();
	void Render();

	void SetScale(float x, float y, float z);
	void SetRotate(bool val) { m_rotate = val; }

	void SetMaterial(Material* pMaterial);

private:
	void CreateCBuffer();

private:
	bool m_rotate;
	Vector3 m_scale;

	CBDataMesh m_cbData;
	UINT m_cbDataByteOffset;
	UINT m_cbDataCBufferPageIndex;
	D3D12_GPU_VIRTUAL_ADDRESS m_cbDataGPUVirtualAddr;

	// 本Demo中 一个网格只有一个材质
	Material* m_pMaterial;

	std::string m_meshGeneratorName;
};

