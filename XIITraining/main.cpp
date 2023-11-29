#include "App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	App* app = new App(hInstance);

	app->Init();

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		// �������Ϣ���ʹ�����Ϣ
		// ���û����Ϣ���ͻ��Ƴ���
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			app->Run();
		}
	}

	app->Release();
	delete app;

	return (int)msg.wParam;
}
