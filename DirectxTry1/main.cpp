#include <Windows.h>
#include "d3dDevice.h"
#include "d3dApp.h"

class MyApp : public D3DApp
{
public:
	// D3DApp��(��) ���� ��ӵ�
	virtual void UpdateScene(float deltaTime) override
	{
		// scene �� �� ������ ����

	}

	virtual void DrawScene() override
	{
		// scene �� �׸���.
		
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