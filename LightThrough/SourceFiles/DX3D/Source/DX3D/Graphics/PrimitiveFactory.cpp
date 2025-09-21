/**
 * @file PrimitiveFactory.cpp
 * @brief 図形生成
 * @author Arima Keita
 * @date 2025-09-12
 */

 // ---------- インクルード ---------- // 
#include <vector>
#include <DX3D/Graphics/PrimitiveFactory.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/Buffers/Vertex.h>


namespace dx3d {
	/**
	 * @brief キューブ生成
	 * @param _device グラフィックスデバイス
	 * @param _size サイズ
	 * @return 生成したメッシュ
	 */
	ecs::Mesh PrimitiveFactory::CreateCube(GraphicsDevice& _device, f32 _size) {
		// [ToDo] Normal未調整
		const Vertex cubeVertices[] = {
			{{-0.5f * _size, -0.5f * _size, -0.5f * _size}, {1.0f, 1.0f, 1.0f, 0.75f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},	// 0
			{{ 0.5f * _size, -0.5f * _size, -0.5f * _size}, {1.0f, 1.0f, 1.0f, 0.75f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},	// 1
			{{ 0.5f * _size,  0.5f * _size, -0.5f * _size}, {1.0f, 1.0f, 1.0f, 0.75f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},	// 2
			{{-0.5f * _size,  0.5f * _size, -0.5f * _size}, {1.0f, 1.0f, 1.0f, 0.75f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},	// 3
			{{-0.5f * _size, -0.5f * _size,  0.5f * _size}, {1.0f, 1.0f, 1.0f, 0.75f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},	// 4
			{{ 0.5f * _size, -0.5f * _size,  0.5f * _size}, {1.0f, 1.0f, 1.0f, 0.75f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},	// 5
			{{ 0.5f * _size,  0.5f * _size,  0.5f * _size}, {1.0f, 1.0f, 1.0f, 0.75f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},	// 6
			{{-0.5f * _size,  0.5f * _size,  0.5f * _size}, {1.0f, 1.0f, 1.0f, 0.75f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},	// 7
		};

		const ui32 cubeIndices[] = {
			0,1,2, 0,2,3,	// 前
			5,4,7, 5,7,6,	// 後
			4,0,3, 4,3,7,	// 左
			1,5,6, 1,6,2,	// 右
			3,2,6, 3,6,7,	// 上
			1,0,4, 1,4,5,	// 下
		};

		auto vb = _device.CreateVertexBuffer({ cubeVertices, std::size(cubeVertices), sizeof(Vertex) });
		auto ib = _device.CreateIndexBuffer({ cubeIndices, std::size(cubeIndices) });

		return ecs::Mesh{ vb, ib };
	}

	/**
	 * @brief クアッド生成
	 * @param _device グラフィックスデバイス
	 * @param _size サイズ
	 * @return 生成したメッシュ
	 */
	ecs::Mesh PrimitiveFactory::CreateQuad(GraphicsDevice& _device, f32 _size)
	{
		const Vertex quadVertices[] = {
			{{-0.5f * _size, -0.5f * _size, 0.0f}, {1.0f, 1.0f, 1.0f, 0.75f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},	// 0
			{{ 0.5f * _size, -0.5f * _size, 0.0f}, {1.0f, 1.0f, 1.0f, 0.75f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},	// 1
			{{ 0.5f * _size,  0.5f * _size, 0.0f}, {1.0f, 1.0f, 1.0f, 0.75f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},	// 2
			{{-0.5f * _size,  0.5f * _size, 0.0f}, {1.0f, 1.0f, 1.0f, 0.75f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},	// 3
		};
		const ui32 quadIndices[] = {
			0, 1, 2,
			0, 2, 3,
		};

		auto vb = _device.CreateVertexBuffer({ quadVertices, std::size(quadVertices), sizeof(Vertex) });
		auto ib = _device.CreateIndexBuffer({ quadIndices, std::size(quadIndices) });

		return ecs::Mesh{ vb, ib };
	}

	/**
	 * @brief スフィア生成
	 * @param _device グラフィックスデバイス
	 * @param _slices 経度方向の分割数
	 * @param _stacks 緯度方向の分割数
	 * @param _radius 半径
	 * @return 生成したメッシュ
	 */
	ecs::Mesh PrimitiveFactory::CreateSphere(GraphicsDevice& _device, ui32 _slices, ui32 _stacks, f32 _radius)
	{
		using namespace DirectX;

		std::vector<Vertex> sphereVertices{};
		// 色
		XMFLOAT4 color{ 1.0f, 1.0f, 1.0f, 0.75f };

		// 頂点生成
		// 北極
		sphereVertices.push_back({ {0.0f, _radius, 0.0f}, {1.0f, 1.0f, 1.0f, 0.75f}, {0.0f, 0.0f }, {0.0f, 1.0f, 0.0f} });

		// 中間
		for (uint32_t i = 1; i <= _stacks; ++i) {
			float phi = XM_PI * i / _stacks; // 緯度角

			for (uint32_t j = 0; j <= _slices; ++j) {
				float theta = 2.0f * XM_PI * j / _slices; // 経度角

				XMFLOAT3 pos{
					_radius * sinf(phi) * cosf(theta),
					_radius * cosf(phi),
					_radius * sinf(phi) * sinf(theta),
				};

				XMFLOAT3 normal{};
				XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&pos)));

				XMFLOAT2 uv{
					theta / (2.0f * XM_PI),
					phi / XM_PI,
				};

				sphereVertices.push_back({ pos, color, uv, normal });
			}
		}

		// 南極
		sphereVertices.push_back({ {0.0f, -_radius, 0.0f}, {1.0f, 1.0f, 1.0f, 0.75f}, {0.0f, 1.0f }, {0.0f, -1.0f, 0.0f} });

		// 北極
		std::vector<ui32> sphereIndices{};
		for (uint32_t i = 1; i <= _slices; ++i) {
			sphereIndices.push_back(0);
			sphereIndices.push_back(i);
			sphereIndices.push_back(i + 1);
		}

		// 中間
		uint32_t baseIndex = 1;
		uint32_t ringVertexCount = _slices + 1;
		for (uint32_t i = 0; i < _stacks - 2; ++i) {
			for (uint32_t j = 0; j < _slices; ++j) {
				sphereIndices.push_back(baseIndex + i * ringVertexCount + j);
				sphereIndices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
				sphereIndices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);

				sphereIndices.push_back(baseIndex + i * ringVertexCount + j);
				sphereIndices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
				sphereIndices.push_back(baseIndex + i * ringVertexCount + j + 1);

			}
		}

		// 南極
		uint32_t southPoleIndex = (uint32_t)sphereVertices.size() - 1;
		baseIndex = southPoleIndex - ringVertexCount;

		for (uint32_t i = 0; i < _slices; ++i) {
			sphereIndices.push_back(southPoleIndex);
			sphereIndices.push_back(baseIndex + i);
			sphereIndices.push_back(baseIndex + i + 1);
		}

		auto vb = _device.CreateVertexBuffer({ sphereVertices.data(), static_cast<ui32>(sphereVertices.size()), sizeof(Vertex) });
		auto ib = _device.CreateIndexBuffer({ sphereIndices.data(), static_cast<ui32>(sphereIndices.size()) });
		return ecs::Mesh{ vb, ib };
	}
}