#include "App.h"

#include <chrono>

class Timer
{
public:
	Timer()
	{
		last = std::chrono::high_resolution_clock::now();
	}

	float Mark()
	{
		const auto old = last;
		last = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<float> frameTime = last - old;
		return frameTime.count();
	}

private:
	std::chrono::steady_clock::time_point last;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_INPUT:
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	AllocConsole();
	FILE* fp = 0;
	freopen_s(&fp, "CONOUT$", "w", stdout);

#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"DirectX12";
	wc.lpszMenuName = 0;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	bool x = RegisterClass(&wc);

	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	g_hWnd = CreateWindow(L"DirectX12", L"DirectX12", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

	ShowWindow(g_hWnd, SW_SHOW);
	UpdateWindow(g_hWnd);

	App* app = new App(width, height);

	app->Init();
	Timer timer;

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		// 如果有消息，就处理消息
		// 如果没有消息，就绘制场景
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			app->Run();

			static float accumTime = 0.0f;
			static int frameCount = 0;

			float frameTime = timer.Mark();
			accumTime += frameTime;
			frameCount++;

			if (accumTime >= 1.0f)
			{
				float fps = frameCount / accumTime;
				frameCount = 0;
				accumTime = 0.0f;

				wchar_t windowText[256];
				swprintf(windowText, sizeof(windowText), L"DirectX12 - FPS: %f", fps);
				SetWindowText(g_hWnd, windowText);
			}
		}
	}

	app->Release();
	delete app;

	return (int)msg.wParam;
}
