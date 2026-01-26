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
		// 24頂点（6面×4）: 各面で頂点を分離し、面法線を設定
		const Vertex cubeVertices[] = {
			// Front (-Z)
			{{-0.5f,-0.5f,-0.5f}, {1,1,1,1}, { 0, 0,-1}, {0,1}}, // 0
			{{ 0.5f,-0.5f,-0.5f}, {1,1,1,1}, { 0, 0,-1}, {1,1}}, // 1
			{{ 0.5f, 0.5f,-0.5f}, {1,1,1,1}, { 0, 0,-1}, {1,0}}, // 2
			{{-0.5f, 0.5f,-0.5f}, {1,1,1,1}, { 0, 0,-1}, {0,0}}, // 3

			// Back (+Z)
			{{-0.5f,-0.5f, 0.5f}, {1,1,1,1}, { 0, 0, 1}, {0,1}}, // 4
			{{ 0.5f,-0.5f, 0.5f}, {1,1,1,1}, { 0, 0, 1}, {1,1}}, // 5
			{{ 0.5f, 0.5f, 0.5f}, {1,1,1,1}, { 0, 0, 1}, {1,0}}, // 6
			{{-0.5f, 0.5f, 0.5f}, {1,1,1,1}, { 0, 0, 1}, {0,0}}, // 7

			// Left (-X)
			{{-0.5f,-0.5f, 0.5f}, {1,1,1,1}, {-1, 0, 0}, {0,1}}, // 8
			{{-0.5f,-0.5f,-0.5f}, {1,1,1,1}, {-1, 0, 0}, {1,1}}, // 9
			{{-0.5f, 0.5f,-0.5f}, {1,1,1,1}, {-1, 0, 0}, {1,0}}, // 10
			{{-0.5f, 0.5f, 0.5f}, {1,1,1,1}, {-1, 0, 0}, {0,0}}, // 11

			// Right (+X)
			{{ 0.5f,-0.5f,-0.5f}, {1,1,1,1}, { 1, 0, 0}, {0,1}}, // 12
			{{ 0.5f,-0.5f, 0.5f}, {1,1,1,1}, { 1, 0, 0}, {1,1}}, // 13
			{{ 0.5f, 0.5f, 0.5f}, {1,1,1,1}, { 1, 0, 0}, {1,0}}, // 14
			{{ 0.5f, 0.5f,-0.5f}, {1,1,1,1}, { 1, 0, 0}, {0,0}}, // 15

			// Top (+Y)
			{{-0.5f, 0.5f,-0.5f}, {1,1,1,1}, { 0, 1, 0}, {0,1}}, // 16
			{{ 0.5f, 0.5f,-0.5f}, {1,1,1,1}, { 0, 1, 0}, {1,1}}, // 17
			{{ 0.5f, 0.5f, 0.5f}, {1,1,1,1}, { 0, 1, 0}, {1,0}}, // 18
			{{-0.5f, 0.5f, 0.5f}, {1,1,1,1}, { 0, 1, 0}, {0,0}}, // 19

			// Bottom (-Y)
			{{-0.5f,-0.5f, 0.5f}, {1,1,1,1}, { 0,-1, 0}, {0,1}}, // 20
			{{ 0.5f,-0.5f, 0.5f}, {1,1,1,1}, { 0,-1, 0}, {1,1}}, // 21
			{{ 0.5f,-0.5f,-0.5f}, {1,1,1,1}, { 0,-1, 0}, {1,0}}, // 22
			{{-0.5f,-0.5f,-0.5f}, {1,1,1,1}, { 0,-1, 0}, {0,0}}, // 23
		};

		// 36インデックス（各面2三角形） - 反時計回り(CCW)に修正
		const uint32_t cubeIndices[] = {
			// 前
			0, 1, 2,  0, 2, 3,
			// 後
			5, 4, 7,  5, 7, 6,
			// 左
			8, 9,10,  8,10,11,
			// 右
			12,13,14, 12,14,15,
			// 上
			16,17,18, 16,18,19,
			// 下
			22,23,20, 22,20,21,
		};

		auto mesh = std::make_shared<Mesh>();
		mesh->vb = _device.CreateVertexBuffer({
			cubeVertices,
			static_cast<uint32_t>(sizeof(cubeVertices)),
			static_cast<uint32_t>(sizeof(Vertex))
			});
		mesh->ib = _device.CreateIndexBuffer({ cubeIndices, static_cast<uint32_t>(std::size(cubeIndices)) });
		mesh->indexCount = static_cast<uint32_t>(std::size(cubeIndices));

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
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},	// 0
			{{ 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},	// 1
			{{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},	// 2
			{{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},	// 3
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
		if (_stacks < 2) { _stacks = 2; }


		// 半径
		const float radius = 0.5f;


		std::vector<Vertex> sphereVertices{};
		// 頂点生成
		// 北極
		sphereVertices.push_back({ {0.0f, radius, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 0.0f} });

		// 中間
		for (uint32_t i = 1; i < _stacks; ++i) {
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
					(float)j / _slices,
					(float)i / _stacks,
				};

				sphereVertices.push_back({ pos, {1.0f, 1.0f, 1.0f, 1.0f}, normal, uv });
			}
		}

		// 南極
		sphereVertices.push_back({ {0.0f, -radius, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.5f, 1.0f } });

		std::vector<uint32_t> sphereIndices{};
		// 北極
		for (uint32_t i = 0; i < _slices; ++i) {
			sphereIndices.push_back(0);
			sphereIndices.push_back(i + 1);
			sphereIndices.push_back(i + 2);
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