#include <Windows.h>
#include "d3dApp.h"

using namespace Acoross::DirectX;

class MyApp : public D3DApp
{
public:
	MyApp(HINSTANCE hInst)
		: D3DApp(hInst, L"MyApp")
	{
	}

	// D3DApp��(��) ���� ��ӵ�
	virtual void UpdateScene(float deltaTime) override
	{
		// scene �� �� ������ ����
	}

	virtual void DrawScene() override
	{
		// scene �� �׸���.
		immediateContext_->ClearRenderTargetView(
			renderTargetView_.Get(),
			reinterpret_cast<const float*>(&Colors::Blue));

		immediateContext_->ClearDepthStencilView(
			depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		HR(swapChain_->Present(0, 0));
	}
};

int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd
)
{
	try
	{
		MyApp app(hInstance);
		app.Run();
	}
	catch (const std::exception&)
	{
		::MessageBox(NULL, L"init failed", L"dd", 0);
		return -1;
	}

	return 0;
}