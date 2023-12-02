#pragma once
#include "header.h"

class Mesh
{
public:
	void InitBox();

	void Update();
	void Render();

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

	Matrix m_mxWorld;
};

