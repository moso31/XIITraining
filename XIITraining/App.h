#pragma once
#include "D3D.h"

class App
{
public:
	App() {}
	App(HINSTANCE hInstance) : m_hInstance(hInstance) {}
	~App() {}

	void Init();

	void Run() {}
	void Release();

private:
	void InitWindow();
	void InitD3D();

private:
	D3D* m_dx = nullptr;
	HINSTANCE m_hInstance = NULL;
	HWND m_hWnd = NULL;

	int m_width = 800;
	int m_height = 600;
};
