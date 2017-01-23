#include "BoxApp.h"
#include <math.h>

using namespace Acoross::DirectX;
using namespace DirectX;

namespace
{
	struct Vertex
	{
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};
}

Acoross::DxCh6::BoxApp::BoxApp(HINSTANCE hInst)
	: D3DApp(hInst, L"MyApp")
{
	XMMATRIX I = ::DirectX::XMMatrixIdentity();
	XMStoreFloat4x4(&world_, I);
	XMStoreFloat4x4(&view_, I);
	XMStoreFloat4x4(&proj_, I);
}

bool Acoross::DxCh6::BoxApp::Init()
{
	if (!D3DApp::Init())
		return false;

	buildGeometryBuffers();
	buildFX();
	buildVertexLayout();

	return true;
}

// D3DApp을(를) 통해 상속됨

void Acoross::DxCh6::BoxApp::OnResize()
{
	D3DApp::OnResize();

	auto p = ::DirectX::XMMatrixPerspectiveFovLH(0.25f * XM_PI, AspectRatio(), 1.0f, 1000.f);
	XMStoreFloat4x4(&proj_, p);
}

void Acoross::DxCh6::BoxApp::UpdateScene(float deltaTime)
{
	float x = radius_ * sinf(phi_) * cosf(theta_);
	float z = radius_ * sinf(phi_) * sinf(theta_);
	float y = radius_ * cosf(phi_);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	const XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = ::DirectX::XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&view_, V);
}

void Acoross::DxCh6::BoxApp::DrawScene()
{
	immediateContext_->ClearRenderTargetView(
		renderTargetView_.Get(),
		reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	immediateContext_->ClearDepthStencilView(
		depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	immediateContext_->IASetInputLayout(inputLayout_.Get());
	immediateContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//useVertexBuffers
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	immediateContext_->IASetVertexBuffers(0, 1, boxVB_.Raw(), &stride, &offset);
	immediateContext_->IASetIndexBuffer(boxIB_.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set constants
	XMMATRIX world = ::DirectX::XMLoadFloat4x4(&world_);
	XMMATRIX view = ::DirectX::XMLoadFloat4x4(&view_);
	XMMATRIX proj = ::DirectX::XMLoadFloat4x4(&proj_);
	XMMATRIX worldViewProj = world * view * proj;

	fxWorldViewProj_->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	D3DX11_TECHNIQUE_DESC techDesc{ 0, };
	tech_->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech_->GetPassByIndex(p)->Apply(0, immediateContext_.Get());

		immediateContext_->DrawIndexed(36, 0, 0);
	}

	HR(swapChain_->Present(0, 0));
}

void Acoross::DxCh6::BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	lastMousePos_.x = x;
	lastMousePos_.y = y;

	::SetCapture(clientDesc_.hMainWnd);
}

void Acoross::DxCh6::BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	::ReleaseCapture();
}

void Acoross::DxCh6::BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - lastMousePos_.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - lastMousePos_.y));

		theta_ += dx;
		phi_ += dy;

		phi_ = Clamp(phi_, 0.1f, XM_PI - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		float dx = XMConvertToRadians(0.005f * static_cast<float>(x - lastMousePos_.x));
		float dy = XMConvertToRadians(0.005f * static_cast<float>(y - lastMousePos_.y));

		radius_ += dx - dy;
		radius_ = Clamp(radius_, 3.0f, 15.0f);
	}

	lastMousePos_.x = x;
	lastMousePos_.y = y;
}

void Acoross::DxCh6::BoxApp::buildGeometryBuffers()
{
	// box 정점 정의
	Vertex box[]
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::White) },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Black) },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Red) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Green) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Blue) },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Yellow) },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Cyan) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Magenta) }
	};

	// 버퍼
	D3D11_BUFFER_DESC boxDesc{ 0, };
	boxDesc.Usage = D3D11_USAGE_IMMUTABLE;
	boxDesc.ByteWidth = sizeof(Vertex) * 8;
	boxDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	boxDesc.CPUAccessFlags = 0;
	boxDesc.MiscFlags = 0;
	boxDesc.StructureByteStride = 0;

	// 버퍼 초기화
	D3D11_SUBRESOURCE_DATA vertexInit{ 0, };
	vertexInit.pSysMem = box;
	HR(d3dDevice_->CreateBuffer(&boxDesc, &vertexInit, boxVB_.Reset()));

	UINT indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,
		// back face
		4, 6, 5,
		4, 7, 6,
		// left face
		4, 5, 1,
		4, 1, 0,
		// right face
		3, 2, 6,
		3, 6, 7,
		// top face
		1, 5, 6,
		1, 6, 2,
		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC idxDesc;
	idxDesc.Usage = D3D11_USAGE_IMMUTABLE;
	idxDesc.ByteWidth = sizeof(UINT) * 36;
	idxDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	idxDesc.CPUAccessFlags = 0;
	idxDesc.MiscFlags = 0;
	idxDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInit{ 0, };
	indexInit.pSysMem = indices;
	HR(d3dDevice_->CreateBuffer(&idxDesc, &indexInit, boxIB_.Reset()));
}

void Acoross::DxCh6::BoxApp::buildFX()
{
	ComObject<ID3D10Blob> compiledShader;
	ComObject<ID3D10Blob> compilationMsgs;
	::D3DCompileFromFile(L"./Common/color.fx", 0, 0, 0, "fx_5_0",
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION,
		0, compiledShader.Reset(), compilationMsgs.Reset());

	::D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(),
		compiledShader->GetBufferSize(), 0, d3dDevice_.Get(), FX_.Reset());

	tech_ = FX_->GetTechniqueByName("ColorTech");
	fxWorldViewProj_ = FX_->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void Acoross::DxCh6::BoxApp::buildVertexLayout()
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3DX11_PASS_DESC passDesc;
	tech_->GetPassByIndex(0)->GetDesc(&passDesc);
	d3dDevice_->CreateInputLayout(
		vertexDesc,
		2,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		inputLayout_.Reset()
	);
}
