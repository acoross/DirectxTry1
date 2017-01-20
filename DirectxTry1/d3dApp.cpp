#include "d3dApp.h"
#include <WindowsX.h>
#include <assert.h>

namespace Acoross {
	namespace DirectX {

		D3DApp* gd3dApp = nullptr;

		LRESULT CALLBACK
			MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			return gd3dApp->MsgProc(hwnd, msg, wParam, lParam);
		}

		D3DApp::D3DApp(HINSTANCE hInstance, std::wstring titleCaption)
			: hAppInst_(hInstance),
			mainWndCaption_(titleCaption)
		{
			gd3dApp = this;

			if (!Init())
			{
				throw std::exception("D3DApp initialization failed");
			}
		}

		bool D3DApp::Init()
		{
			if (!InitMainWnd())
			{
				return false;
			}

			if (!InitD3D())
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

				if (runtimeOption_.Initialized)
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
						OnResize();
					}
					else if (wParam == SIZE_RESTORED)
					{
						// Restoring from minimized state?
						if (minimized_)
						{
							appPaused_ = false;
							minimized_ = false;
							OnResize();
						}

						// Restoring from maximized state?
						else if (maximized_)
						{
							appPaused_ = false;
							maximized_ = false;
							OnResize();
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
							OnResize();
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
				OnResize();
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

		bool D3DApp::InitMainWnd()
		{
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

			return true;
		}

		bool D3DApp::InitD3D()
		{
			if (!createD3dDevice())
			{
				return false;
			}

			UINT m4xMsaaQuality;
			HRESULT hr = d3dDevice_->CheckMultisampleQualityLevels(
				DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);
			if (FAILED(hr))
			{
				::MessageBox(0, L"Msaa quality check failed.", 0, 0);
				return false;
			}
			runtimeOption_.m4xMsaaQuality = m4xMsaaQuality;
			assert(m4xMsaaQuality > 0);

			if (!initSwapChain())
			{
				return false;
			}

			resetRenderTargetView();

			// Create the depth/stencil buffer and view.
			createStencilBufferView();

			// Bind the render target view and depth/stencil view to the pipeline.
			bindTargetViewAndStencilBuffer();

			// Set the viewport transform.
			setViewPortTransform();

			runtimeOption_.Initialized = true;
			return true;
		}

		void D3DApp::OnResize()
		{
			assert(immediateContext_.Get());
			assert(d3dDevice_.Get());
			assert(swapChain_.Get());

			renderTargetView_.Release();
			depthStencilView_.Release();
			depthStencilBuffer_.Release();

			// Resize the swap chain and recreate the render target view.
			HR(swapChain_->ResizeBuffers(1, clientDesc_.Width, clientDesc_.Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

			resetRenderTargetView();

			// Create the depth/stencil buffer and view.
			createStencilBufferView();

			// Bind the render target view and depth/stencil view to the pipeline.
			bindTargetViewAndStencilBuffer();

			// Set the viewport transform.
			setViewPortTransform();
		}

		bool D3DApp::createD3dDevice()
		{
			D3D_FEATURE_LEVEL featureLevel;

			HRESULT hr = ::D3D11CreateDevice(
				NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG,
				NULL,
				0,
				D3D11_SDK_VERSION,
				d3dDevice_.Reset(),
				&featureLevel,
				immediateContext_.Reset());

			if (FAILED(hr))
			{
				::MessageBox(0, L"D3D11CreateDevice failed", 0, 0);
				return false;
			}

			if (featureLevel != D3D_FEATURE_LEVEL_11_0)
			{
				::MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
				return false;
			}

			return true;
		}

		bool D3DApp::initSwapChain()
		{
			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			swapChainDesc.BufferDesc.Width = clientDesc_.Width;
			swapChainDesc.BufferDesc.Height = clientDesc_.Height;
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			if (runtimeOption_.Enable4xMsaa)
			{
				swapChainDesc.SampleDesc.Count = 4; // dx11 지원이므로 반드시 4X MSAA 사용 가능하다. 
				swapChainDesc.SampleDesc.Quality = runtimeOption_.m4xMsaaQuality - 1;
			}
			else
			{
				swapChainDesc.SampleDesc.Count = 1;
				swapChainDesc.SampleDesc.Quality = 0;
			}

			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 1;
			swapChainDesc.OutputWindow = clientDesc_.hMainWnd;
			swapChainDesc.Windowed = true;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDesc.Flags = 0;

			auto dxgiDevice = make_com<IDXGIDevice>(nullptr);
			if (FAILED(d3dDevice_->QueryInterface(__uuidof(IDXGIDevice), (void**)dxgiDevice.Reset())))
			{
				return false;
			}

			auto dxgiAdapter = make_com<IDXGIAdapter>(nullptr);
			if (FAILED(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)dxgiAdapter.Reset())))
			{
				return false;
			}

			auto dxgiFactory = make_com<IDXGIFactory>(nullptr);
			if (FAILED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)dxgiFactory.Reset())))
			{
				return false;
			}

			if (FAILED(dxgiFactory->CreateSwapChain(d3dDevice_.Get(), &swapChainDesc, swapChain_.Reset())))
			{
				return false;
			}

			return true;
		}

		// swap chain 의 back buffer 를 받아 renderTargetView 를 만든다. 이 renderTargetView 는 d3dDevice 에 연결되어 있다.
		bool D3DApp::resetRenderTargetView()
		{
			auto backBuffer = make_com<ID3D11Texture2D>(nullptr);
			HR(swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.Reset())));
			HR(d3dDevice_->CreateRenderTargetView(backBuffer.Get(), 0, renderTargetView_.Reset()));
			
			return true;
		}

		bool D3DApp::createStencilBufferView()
		{
			D3D11_TEXTURE2D_DESC depthStencilDesc{ 0, };
			depthStencilDesc.Width = clientDesc_.Width;
			depthStencilDesc.Height = clientDesc_.Height;
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.ArraySize = 1;
			depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

			if (runtimeOption_.Enable4xMsaa)
			{
				depthStencilDesc.SampleDesc.Count = 4;
				depthStencilDesc.SampleDesc.Quality = runtimeOption_.m4xMsaaQuality - 1;
			}
			else
			{
				depthStencilDesc.SampleDesc.Count = 1;
				depthStencilDesc.SampleDesc.Quality = 0;
			}

			depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
			depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			depthStencilDesc.CPUAccessFlags = 0;
			depthStencilDesc.MiscFlags = 0;

			if (FAILED(d3dDevice_->CreateTexture2D(
				&depthStencilDesc,
				0,
				depthStencilBuffer_.Reset())))
			{
				return false;
			}

			if (FAILED(d3dDevice_->CreateDepthStencilView(
				depthStencilBuffer_.Get(),
				0,
				depthStencilView_.Reset())))
			{
				return false;
			}

			return true;
		}

		bool D3DApp::bindTargetViewAndStencilBuffer()
		{
			// Bind the render target view and depth/stencil view to the pipeline.
			immediateContext_->OMSetRenderTargets(1, renderTargetView_.Raw(), depthStencilView_.Get());
			return true;
		}

		bool D3DApp::setViewPortTransform()
		{
			// Set the viewport transform.
			D3D11_VIEWPORT screenViewport_{ 0, };
			screenViewport_.TopLeftX = 0;
			screenViewport_.TopLeftY = 0;
			screenViewport_.Width = static_cast<float>(clientDesc_.Width);
			screenViewport_.Height = static_cast<float>(clientDesc_.Height);
			screenViewport_.MinDepth = 0.0f;
			screenViewport_.MaxDepth = 1.0f;

			immediateContext_->RSSetViewports(1, &screenViewport_);
			return true;
		}
	}
}