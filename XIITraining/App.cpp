#include "App.h"

void App::Init()
{
	InitD3D();
}

void App::Run()
{
	g_dx->Update();
	g_dx->Render();
}

void App::Release()
{
	g_dx->Release();
	g_dx = nullptr;
}

void App::InitD3D()
{
	g_dx = new D3D(m_width, m_height);
	g_dx->Init();
}
