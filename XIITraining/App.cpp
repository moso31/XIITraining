#include "App.h"

void App::Init()
{
	InitWindow();
	InitD3D();
}

void App::Run()
{
	g_dx->Render();
}

void App::Release()
{
	g_dx->Release();
	g_dx = nullptr;
}

void App::InitWindow()
{
	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DefWindowProc;
	wc.hInstance = m_hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"DirectX12";
	wc.lpszMenuName = 0;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	bool x = RegisterClass(&wc);

	RECT rc = { 0, 0, m_width, m_height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	auto k = GetLastError();

	m_hWnd = CreateWindow(L"DirectX12", L"DirectX12", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, m_hInstance, NULL);

	auto k2 = GetLastError();

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
}

void App::InitD3D()
{
	g_dx = new D3D(m_width, m_height, m_hWnd);
	g_dx->Init();
}
