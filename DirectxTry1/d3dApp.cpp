#include "d3dApp.h"
#include <WindowsX.h>

namespace
{
	D3DApp* gd3dApp = 0;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return gd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

bool D3DApp::Init()
{
	gd3dApp = this;

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hAppInst_;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, (long)clientDesc_.Width, (long)clientDesc_.Height };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	clientDesc_.hMainWnd = CreateWindow(L"D3DWndClassName", mainWndCaption_.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, hAppInst_, 0);
	if (!clientDesc_.hMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(clientDesc_.hMainWnd, SW_SHOW);
	UpdateWindow(clientDesc_.hMainWnd);

	if (!d3dDevice_.InitD3D(clientDesc_.hMainWnd, clientDesc_.Width, clientDesc_.Height))
	{
		return false;
	}

	return true;
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			appPaused_ = true;
			timer_.Stop();
		}
		else
		{
			appPaused_ = false;
			timer_.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		clientDesc_.Width = LOWORD(lParam);
		clientDesc_.Height = HIWORD(lParam);
		
		if (d3dDevice_.Initialized)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				appPaused_ = true;
				minimized_ = true;
				maximized_ = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				appPaused_ = false;
				minimized_ = false;
				maximized_ = true;
				d3dDevice_.OnResize(clientDesc_.Width, clientDesc_.Height);
			}
			else if (wParam == SIZE_RESTORED)
			{
				// Restoring from minimized state?
				if (minimized_)
				{
					appPaused_ = false;
					minimized_ = false;
					d3dDevice_.OnResize(clientDesc_.Width, clientDesc_.Height);
				}

				// Restoring from maximized state?
				else if (maximized_)
				{
					appPaused_ = false;
					maximized_ = false;
					d3dDevice_.OnResize(clientDesc_.Width, clientDesc_.Height);
				}
				else if (resizing_)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					d3dDevice_.OnResize(clientDesc_.Width, clientDesc_.Height);
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		appPaused_ = true;
		resizing_ = true;
		timer_.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		appPaused_ = false;
		resizing_ = false;
		timer_.Start();
		d3dDevice_.OnResize(clientDesc_.Width, clientDesc_.Height);
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int D3DApp::Run()
{
	timer_.Reset();

	MSG msg{ 0 };
	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			timer_.Tick();

			if (!appPaused_)
			{
				UpdateScene(timer_.DeltaTime());
				DrawScene();
			}
			else
			{
				::Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}
