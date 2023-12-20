#pragma once
#include "header.h"

class Material;
class Mesh
{
public:
	void InitBox();

	void Update();
	void Render();

	void SetScale(float x, float y, float z);
	void SetRotate(bool val) { m_rotate = val; }

	void SetMaterial(Material* pMaterial);

private:
	void CreateVB();
	void CreateIB();

private:
	std::vector<VertexPNTC> m_vertices;
	std::vector<UINT> m_indices;

	ComPtr<ID3D12Resource> m_pVB;
	ComPtr<ID3D12Resource> m_pIB;
	ComPtr<ID3D12Resource> m_pVBUpload;
	ComPtr<ID3D12Resource> m_pIBUpload;

	bool m_rotate;
	Vector3 m_scale;

	// 本Demo中 一个网格只有一个材质
	Material* m_pMaterial;
};

