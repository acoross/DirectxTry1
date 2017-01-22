#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <DirectXMath.h>
#include "GameTimer.h"
#include "ComObject.h"

#define HR(x) Acoross::DirectX::_HR(x, TEXT(__FILE__), __LINE__)

namespace Acoross {
	namespace DirectX {

		namespace Colors
		{
			XMGLOBALCONST ::DirectX::XMVECTORF32 White = { 1.0f, 1.0f, 1.0f, 1.0f };
			XMGLOBALCONST ::DirectX::XMVECTORF32 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
			XMGLOBALCONST ::DirectX::XMVECTORF32 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
			XMGLOBALCONST ::DirectX::XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
			XMGLOBALCONST ::DirectX::XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
			XMGLOBALCONST ::DirectX::XMVECTORF32 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
			XMGLOBALCONST ::DirectX::XMVECTORF32 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
			XMGLOBALCONST ::DirectX::XMVECTORF32 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
			XMGLOBALCONST ::DirectX::XMVECTORF32 Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
			XMGLOBALCONST ::DirectX::XMVECTORF32 LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };
		}

		inline void _HR(HRESULT hr, std::wstring filename, int line)
		{
			if (FAILED(hr))
			{
				::MessageBox(NULL, L"dd", L"dd", 0);

				wchar_t buf[100]{ 0, };
				swprintf_s(buf, L"%s, %d\n", filename.c_str(), line);
				OutputDebugString(buf);
			}
		}

		inline float Clamp(float x, float low, float high)
		{
			return x < low ? low : (x > high ? high : x);
		}

		class D3DApp
		{
		public:
			D3DApp(HINSTANCE hInstance, std::wstring titleCaption);
			int Run();

			LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

			virtual bool Init();
			
		protected:
			virtual void OnResize();
			virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
			virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
			virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

			virtual void UpdateScene(float deltaTime) = 0;
			virtual void DrawScene() = 0;

			bool InitMainWnd();
			bool InitD3D();

			bool createD3dDevice();
			bool initSwapChain();
			bool resetRenderTargetView();
			bool createStencilBufferView();
			bool bindTargetViewAndStencilBuffer();
			bool setViewPortTransform();

			float AspectRatio() const;

			GameTimer timer_;

			struct ClientDesc {
				UINT Width;
				UINT Height;
				HWND hMainWnd;
			};
			ClientDesc clientDesc_{ 800, 600, false };

			const std::wstring mainWndCaption_;
			const HINSTANCE hAppInst_;

			bool appPaused_{ false };
			bool resizing_{ false };
			bool minimized_{ true };
			bool maximized_{ false };

			// d3d
			struct RuntimeOption {
				bool Enable4xMsaa;
				UINT m4xMsaaQuality;
				bool Initialized;
			};
			RuntimeOption runtimeOption_{ false, 0, false };

			ComObject<ID3D11Device> d3dDevice_{ nullptr };
			ComObject<ID3D11DeviceContext> immediateContext_{ nullptr };
			ComObject<IDXGISwapChain> swapChain_{ nullptr };
			ComObject<ID3D11RenderTargetView> renderTargetView_{ nullptr };
			ComObject<ID3D11Texture2D> depthStencilBuffer_{ nullptr };
			ComObject<ID3D11DepthStencilView> depthStencilView_{ nullptr };
		};
	}
}