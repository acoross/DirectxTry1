#include <Windows.h>
#include "d3dDevice.h"
#include "d3dApp.h"

class MyApp : public D3DApp
{
public:
	// D3DApp을(를) 통해 상속됨
	virtual void UpdateScene(float deltaTime) override
	{
		// scene 에 들어갈 내용을 갱신

	}

	virtual void DrawScene() override
	{
		// scene 을 그린다.
		
	}
};

int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd
)
{
	MyApp app;
	app.Init();
	app.Run();
}