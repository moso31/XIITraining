#pragma once
#include "D3D.h"

class App
{
public:
	App(int w, int h) : m_width(w), m_height(h) {}
	~App() {}

	void Init();

	void Run();
	void Release();

private:
	void InitD3D();

private:
	int m_width = 800;
	int m_height = 600;
};
