#pragma once

#include <Windows.h>
#include <string>
#include "d3dDevice.h"
#include "GameTimer.h"

class D3DApp
{
public:
	bool Init();
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	
	int Run();

	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

	virtual void UpdateScene(float deltaTime) = 0;
	virtual void DrawScene() = 0;

protected:
	D3DDevice d3dDevice_;
	GameTimer timer_;

private:
	struct ClientDesc {
		UINT Width;
		UINT Height;
		HWND hMainWnd;
	};
	ClientDesc clientDesc_{ 0, };

	std::wstring mainWndCaption_;
	HINSTANCE hAppInst_;

	bool appPaused_{ false };
	bool resizing_{ false };
	bool minimized_{ true };
	bool maximized_{ false };
};