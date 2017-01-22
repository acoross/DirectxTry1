#include <Windows.h>
#include "BoxApp.h"

using Acoross::DxCh6::BoxApp;

int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	try
	{
		BoxApp app(hInstance);
		if (app.Init())
			app.Run();
	}
	catch (const std::exception&)
	{
		::MessageBox(NULL, L"init failed", L"dd", 0);
		return -1;
	}

	return 0;
}