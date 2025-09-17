/**
 * @file PrimitiveFactory.cpp
 * @brief 図形生成
 * @author Arima Keita
 * @date 2025-09-12
 */

// ---------- インクルード ---------- // 
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
	ecs::Mesh PrimitiveFactory::CreateCube(GraphicsDevice& _device, float _size) {
		// [ToDo] Normal未調整
		const Vertex cubeVertices[] = {
			{{-0.5f * _size, -0.5f * _size, -0.5f * _size}, { 1, 1, 1, 0.75f}, {0, 1}, {1, 0, 0}},	// 0
			{{-0.5f * _size,  0.5f * _size, -0.5f * _size}, { 1, 1, 1, 0.75f}, {0, 0}, {1, 0, 0}},	// 1
			{{ 0.5f * _size,  0.5f * _size, -0.5f * _size}, { 1, 1, 1, 0.75f}, {1, 0}, {1, 0, 0}},	// 2
			{{ 0.5f * _size, -0.5f * _size, -0.5f * _size}, { 1, 1, 1, 0.75f}, {1, 1}, {1, 0, 0}},	// 3
			{{-0.5f * _size, -0.5f * _size,  0.5f * _size}, { 1, 1, 1, 0.75f}, {0, 1}, {1, 0, 0}},	// 4
			{{-0.5f * _size,  0.5f * _size,  0.5f * _size}, { 1, 1, 1, 0.75f}, {0, 0}, {1, 0, 0}},	// 5
			{{ 0.5f * _size,  0.5f * _size,  0.5f * _size}, { 1, 1, 1, 0.75f}, {1, 0}, {1, 0, 0}},	// 6
			{{ 0.5f * _size, -0.5f * _size,  0.5f * _size}, { 1, 1, 1, 0.75f}, {1, 1}, {1, 0, 0}},	// 7
		};

		const ui32 cubeIndices[] = {
			0,1,2, 0,2,3,	// 前
			4,6,5, 4,7,6,	// 後
			4,5,1, 4,1,0,	// 左
			3,2,6, 3,6,7,	// 右
			1,5,6, 1,6,2,	// 上
			4,0,3, 4,3,7,	// 下
		};

		auto vb = _device.CreateVertexBuffer({ cubeVertices, std::size(cubeVertices), sizeof(Vertex) });
		auto ib = _device.CreateIndexBuffer({ cubeIndices, std::size(cubeIndices) });

		return ecs::Mesh{ vb, ib };
	}
	ecs::Mesh PrimitiveFactory::CreateQuad(GraphicsDevice& _device, float _size)
	{

		const Vertex quadVertices[] = {
			{{-0.5f * _size, -0.5f * _size, 0.0f}, {1, 1, 1, 0.75f}, {0, 1}, {0, 0, -1}},	// 0
			{{-0.5f * _size,  0.5f * _size, 0.0f}, {1, 1, 1, 0.75f}, {1, 1}, {0, 0, -1}},	// 1
			{{ 0.5f * _size,  0.5f * _size, 0.0f}, {1, 1, 1, 0.75f}, {1, 0}, {0, 0, -1}},	// 2
			{{ 0.5f * _size, -0.5f * _size, 0.0f}, {1, 1, 1, 0.75f}, {0, 0}, {0, 0, -1}},	// 3
		};

		const ui32 quadIndices[] = {
			0, 1, 2,
			0, 2, 3,
		};
		

		auto vb = _device.CreateVertexBuffer({ quadVertices, std::size(quadVertices), sizeof(Vertex) });
		auto ib = _device.CreateIndexBuffer({ quadIndices, std::size(quadIndices) });

		return ecs::Mesh{ vb, ib };
	}
}