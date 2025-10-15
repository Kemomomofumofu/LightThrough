/**
 * @file PrimitiveFactory.cpp
 * @brief 図形生成
 * @author Arima Keita
 * @date 2025-09-12
 */

 // ---------- インクルード ---------- // 
#include <DirectXMath.h>
#include <DX3D/Graphics/Meshes/PrimitiveFactory.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/Buffers/Vertex.h>
#include <DX3D/Graphics/Meshes/MeshRegistry.h>
#include <DX3D/Graphics/Meshes/MeshHandle.h>
#include <DX3D/Graphics/Meshes/Mesh.h>


namespace dx3d {
	/**
	 * @brief キューブ生成
	 * @param _device グラフィックスデバイス
	 * @param _registry メッシュレジストリ
	 * @return 生成したメッシュのハンドル
	 */
	MeshHandle PrimitiveFactory::CreateCube(GraphicsDevice& _device, MeshRegistry& _registry)
	{


		// [ToDo] Normal未調整
		const Vertex cubeVertices[] = {
			{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},	// 0
			{{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},	// 1
			{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},	// 2
			{{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},	// 3

			{{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},	// 4
			{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},	// 5
			{{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},	// 6
			{{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},	// 7
		};

		const uint32_t cubeIndices[] = {
			0,1,2, 0,2,3,	// 前
			5,4,7, 5,7,6,	// 後
			4,0,3, 4,3,7,	// 左
			1,5,6, 1,6,2,	// 右
			3,2,6, 3,6,7,	// 上
			1,0,4, 1,4,5,	// 下
		};

		// メッシュの作成
		auto mesh = std::make_shared<Mesh>();
		mesh->vb = _device.CreateVertexBuffer({
			cubeVertices,
			static_cast<uint32_t>(sizeof(cubeVertices)),
			static_cast<uint32_t>(sizeof(Vertex))
			});
		mesh->ib = _device.CreateIndexBuffer({ cubeIndices, std::size(cubeIndices) });
		mesh->indexCount = std::size(cubeIndices);


		return _registry.Register(mesh, "Cube");
	}

	/**
	 * @brief クアッド生成
	 * @param _device グラフィックスデバイス
	 * @param _registry メッシュレジストリ
	 * @return 生成したメッシュのハンドル
	 */
	MeshHandle PrimitiveFactory::CreateQuad(GraphicsDevice& _device, MeshRegistry& _registry)
	{
			const Vertex quadVertices[] = {
				{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},	// 0
				{{ 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},	// 1
				{{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},	// 2
				{{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},	// 3
			};
			const uint32_t quadIndices[] = {
				0, 1, 2,
				0, 2, 3,
			};

			// メッシュの作成
			auto mesh = std::make_shared<Mesh>();
			mesh->vb = _device.CreateVertexBuffer({
				quadVertices,
				static_cast<uint32_t>(std::size(quadVertices) * sizeof(Vertex)),
				static_cast<uint32_t>(sizeof(Vertex))
				});
			mesh->ib = _device.CreateIndexBuffer({ quadIndices, std::size(quadIndices) });
			mesh->indexCount = std::size(quadIndices);


			return _registry.Register(mesh, "Quad");
	}

	/**
	 * @brief スフィア生成
	 * @param _device グラフィックスデバイス
	 * @param _registry メッシュレジストリ
	 * @param _slices 経度方向の分割数
	 * @param _stacks 緯度方向の分割数
	 * @param _radius 半径
	 * @return 生成したメッシュのハンドル
	 */
	MeshHandle PrimitiveFactory::CreateSphere(GraphicsDevice& _device, MeshRegistry& _registry, uint32_t _slices, uint32_t _stacks)
	{
		using namespace DirectX;

		if (_slices < 3) { _slices = 3; }
		if (_stacks < 3) { _stacks = 3; }


		// 半径
		const float radius = 0.5f;


		std::vector<Vertex> sphereVertices{};
		// 頂点生成
		// 北極
		sphereVertices.push_back({ {0.0f, radius, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}});

		// 中間
		for (uint32_t i = 1; i <= _stacks; ++i) {
			float phi = XM_PI * i / _stacks; // 緯度角

			for (uint32_t j = 0; j <= _slices; ++j) {
				float theta = 2.0f * XM_PI * j / _slices; // 経度角

				XMFLOAT3 pos{
					radius * sinf(phi) * cosf(theta),
					radius * cosf(phi),
					radius * sinf(phi) * sinf(theta),
				};

				XMFLOAT3 normal{};
				XMStoreFloat3(&normal, XMVector3Normalize(XMLoadFloat3(&pos)));

				XMFLOAT2 uv{
					theta / (2.0f * XM_PI),
					phi / XM_PI,
				};

				sphereVertices.push_back({ pos, {1.0f, 1.0f, 1.0f, 1.0f}, normal, uv });
			}
		}

		// 南極
		sphereVertices.push_back({ {0.0f, -radius, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f } });

		// 北極
		std::vector<uint32_t> sphereIndices{};
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

		// [ToDo] ここだけ描画されていない。
		for (uint32_t i = 0; i < _slices; ++i) {
			sphereIndices.push_back(southPoleIndex);
			sphereIndices.push_back(baseIndex + i + 1);
			sphereIndices.push_back(baseIndex + i);
		}

		auto mesh = std::make_shared<Mesh>();

		mesh->vb = _device.CreateVertexBuffer({
			sphereVertices.data(),
			static_cast<uint32_t>(sphereVertices.size() * sizeof(Vertex)),
			static_cast<uint32_t>(sizeof(Vertex))
			});
		mesh->ib = _device.CreateIndexBuffer({ sphereIndices.data(), (uint32_t)sphereIndices.size() });
		mesh->indexCount = (uint32_t)sphereIndices.size();

		return _registry.Register(mesh, "Sphere");
	}
}