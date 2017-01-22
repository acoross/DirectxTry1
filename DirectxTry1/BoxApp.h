#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <D3Dcompiler.h>
#include "../FX11/inc/d3dx11effect.h"
#include "Common/d3dApp.h"

using DirectX::XMFLOAT4X4;
using Acoross::DirectX::ComObject;

namespace Acoross
{
	namespace DxCh6
	{
		class BoxApp : public Acoross::DirectX::D3DApp
		{
		public:
			BoxApp(HINSTANCE hInst);
			virtual bool Init() override;

		protected:
			virtual void OnResize() override;
			virtual void UpdateScene(float deltaTime) override;
			virtual void DrawScene() override;
			virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
			virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
			virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

		private:
			void buildGeometryBuffers();
			void buildFX();
			void buildVertexLayout();

			ComObject<ID3D11Buffer> boxVB_{ nullptr };
			ComObject<ID3D11Buffer> boxIB_{ nullptr };

			ComObject<ID3DX11Effect> FX_{ nullptr };
			ID3DX11EffectTechnique* tech_{ nullptr };	// com 객체가 아니다.
			ID3DX11EffectMatrixVariable* fxWorldViewProj_{ nullptr }; // com 객체가 아니다.

			ComObject<ID3D11InputLayout> inputLayout_{ nullptr };

			XMFLOAT4X4 world_;
			XMFLOAT4X4 view_;
			XMFLOAT4X4 proj_;

			POINT lastMousePos_{ 0, };

			float theta_{ 1.5f * ::DirectX::XM_PI };
			float phi_{ 0.25f * ::DirectX::XM_PI };
			float radius_{ 5.f };
		};
	}
}