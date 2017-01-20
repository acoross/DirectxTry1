#include "d3dDevice.h"

bool D3DDevice::InitD3D(HWND hWnd, UINT width, UINT height)
{
	clientDesc_.hMainWnd = hWnd;
	clientDesc_.Width = width;
	clientDesc_.Height = height;

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
	
	Initialized = true;
	return true;
}

void D3DDevice::OnResize(UINT width, UINT height)
{
	clientDesc_.Width = width;
	clientDesc_.Height = height;

	assert(immediateContext_.Get());
	assert(d3dDevice_.Get());
	assert(swapChain_.Get());
	
	// Resize the swap chain and recreate the render target view.
	HR(swapChain_->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	resetRenderTargetView();
	
	// Create the depth/stencil buffer and view.
	createStencilBufferView();

	// Bind the render target view and depth/stencil view to the pipeline.
	bindTargetViewAndStencilBuffer();

	// Set the viewport transform.
	setViewPortTransform();
}

bool D3DDevice::createD3dDevice()
{
	ID3D11Device* d3dDevice{ nullptr };
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11DeviceContext* immediateContext{ nullptr };

	HRESULT hr = D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&d3dDevice,
		&featureLevel,
		&immediateContext);

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

	d3dDevice_ = make_com(d3dDevice);
	immediateContext_ = make_com(immediateContext);
	
	return true;
}

bool D3DDevice::initSwapChain()
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

	IDXGIDevice* dxgiDevice = nullptr;
	if (FAILED(d3dDevice_->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice)))
	{
		return false;
	}

	IDXGIAdapter* dxgiAdapter = nullptr;
	if (FAILED(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter)))
	{
		return false;
	}

	IDXGIFactory* dxgiFactory = nullptr;
	if (FAILED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory)))
	{
		return false;
	}

	IDXGISwapChain* swapChain{ nullptr };
	if (FAILED(dxgiFactory->CreateSwapChain(d3dDevice_.Get(), &swapChainDesc, &swapChain)))
	{
		return false;
	}
	
	ReleaseCom(&dxgiDevice);
	ReleaseCom(&dxgiAdapter);
	ReleaseCom(&dxgiFactory);

	swapChain_ = make_com(swapChain);

	return true;
}

bool D3DDevice::resetRenderTargetView()
{
	ID3D11RenderTargetView* renderTargetView{ nullptr };

	ID3D11Texture2D* backBuffer;
	HR(swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(d3dDevice_->CreateRenderTargetView(backBuffer, 0, &renderTargetView));
	ReleaseCom(&backBuffer);

	renderTargetView_ = make_com(renderTargetView);
	return true;
}

bool D3DDevice::createStencilBufferView()
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

	ID3D11Texture2D* depthStencilBuffer{ nullptr };
	if (FAILED(d3dDevice_->CreateTexture2D(
		&depthStencilDesc,
		0,
		&depthStencilBuffer)))
	{
		return false;
	}
	depthStencilBuffer_ = make_com(depthStencilBuffer);

	ID3D11DepthStencilView* depthStencilView{ nullptr };
	if (FAILED(d3dDevice_->CreateDepthStencilView(
		depthStencilBuffer_.Get(),
		0,
		&depthStencilView)))
	{
		return false;
	}
	depthStencilView_ = make_com(depthStencilView);

	return true;
}

bool D3DDevice::bindTargetViewAndStencilBuffer()
{
	// Bind the render target view and depth/stencil view to the pipeline.
	immediateContext_->OMSetRenderTargets(1, renderTargetView_.Raw(), depthStencilView_.Get());
	return true;
}

bool D3DDevice::setViewPortTransform()
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
