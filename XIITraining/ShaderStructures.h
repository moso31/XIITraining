#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

struct VertexPNTC
{
	VertexPNTC(const Vector3& pos, const Vector3& norm, const Vector2& uv, const Vector4& col) : pos(pos), norm(norm), uv(uv), col(col) {}

	Vector3 pos = Vector3(0.0f);
	Vector3 norm = Vector3(0.0f);
	Vector2 uv = Vector2(0.0f);
	Vector4 col = Vector4(0.0f);
};

struct CBufferPerFrame
{
	Matrix m_view;
	Matrix m_proj;
};

struct CBufferPerObject
{
	Matrix m_world;
};
