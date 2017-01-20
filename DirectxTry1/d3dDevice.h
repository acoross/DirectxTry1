#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <utility>
#include "ComObject.h"

inline void HR(HRESULT hr)
{

}

class D3DDevice
{
public:
	D3DDevice()
	{}

	~D3DDevice()
	{}

	bool InitD3D(HWND hWnd, UINT width, UINT height);
	void OnResize(UINT width, UINT height);

	bool Initialized{ false };

private:
	bool createD3dDevice();
	bool initSwapChain();
	bool resetRenderTargetView();
	bool createStencilBufferView();
	bool bindTargetViewAndStencilBuffer();
	bool setViewPortTransform();

private:
	struct ClientDesc {
		UINT Width;
		UINT Height;
		HWND hMainWnd;
	};
	ClientDesc clientDesc_{ 0, };

	struct RuntimeOption {
		bool Enable4xMsaa;
		UINT m4xMsaaQuality;
	};
	RuntimeOption runtimeOption_{ 0, };

	ComObject<ID3D11Device> d3dDevice_{ nullptr };
	ComObject<ID3D11DeviceContext> immediateContext_{ nullptr };
	ComObject<IDXGISwapChain> swapChain_{ nullptr };
	ComObject<ID3D11RenderTargetView> renderTargetView_{ nullptr };
	ComObject<ID3D11Texture2D> depthStencilBuffer_{ nullptr };
	ComObject<ID3D11DepthStencilView> depthStencilView_{ nullptr };
};