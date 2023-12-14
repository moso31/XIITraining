#pragma once
#include "header.h"

class Mesh
{
public:
	void InitBox();

	void Update();
	void Render();

	void SetScale(float x, float y, float z);
	void SetRotate(bool val) { m_rotate = val; }

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
};

