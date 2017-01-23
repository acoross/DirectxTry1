#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <exception>
#include <utility>
#include <vector>

using DirectX::XMFLOAT3;

namespace Acoross
{
	namespace DxCh6
	{
		class GeometryGenerator
		{
		public:
			struct Vertex
			{
				Vertex() {}

				XMFLOAT3 Position;
				/*XMFLOAT3 Normal;
				XMFLOAT3 TangentU;
				XMFLOAT3 TexC;*/
			};

			struct MeshData
			{
				std::vector<Vertex> Vertices;
				std::vector<UINT> Indices;
			};

			// m: depth, n: width
			void CreateGrid(float depth, float width, UINT m, UINT n, OUT MeshData& mesh)
			{
				if (width <= 0 || depth <= 0) throw std::exception();
				
				UINT count = m * n;

				float halfDepth = depth * .5f;
				float halfWidth = width * .5f;
				float dz = depth / (m - 1);
				float dx = width / (n - 1);

				mesh.Vertices.resize(count);

				for (UINT i = 0; i < m; ++i)
				{
					float z = halfDepth - i * dz;
					for (UINT j = 0; j < n; ++j)
					{
						float x = -halfWidth + j * dx;
						mesh.Vertices[i * n + j].Position = XMFLOAT3(x, 0.0f, z);
					}
				}

				UINT faceCount = (m - 1) * (n - 1) * 2; // 격자마다 삼각형 2개
				mesh.Indices.resize(faceCount * 3);	// 면 마다 점 3개

				int k = 0;
				for (UINT i = 0; i < m; ++i)
				{
					for (UINT j = 0; j < n; ++j)
					{
						mesh.Indices[k] = i * n + j;
						mesh.Indices[k + 1] = i * n + j + 1;
						mesh.Indices[k + 2] = (i + 1) * n + j;
						mesh.Indices[k + 3] = (i + 1) * n + j;
						mesh.Indices[k + 4] = i * n + j + 1;
						mesh.Indices[k + 5] = (i + 1) * n + j + 1;
						k += 6;
					}
				}
			}
		};
	}
}