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
	ecs::Mesh PrimitiveFactory::CreateCube(GraphicsDevice& _device, float _size) {
		const Vertex cubeVertices[] = {
			{{-0.5f, -0.5f, -0.5f}, { 1, 1, 1, 1}, {0, 1}},	// 0
			{{-0.5f,  0.5f, -0.5f}, { 1, 1, 1, 1}, {0, 0}},	// 1
			{{ 0.5f,  0.5f, -0.5f}, { 1, 1, 1, 1}, {1, 0}},	// 2
			{{ 0.5f, -0.5f, -0.5f}, { 1, 1, 1, 1}, {1, 1}},	// 3
			{{-0.5f, -0.5f,  0.5f}, { 1, 1, 1, 1}, {0, 1}},	// 4
			{{-0.5f,  0.5f,  0.5f}, { 1, 1, 1, 1}, {0, 0}},	// 5
			{{ 0.5f,  0.5f,  0.5f}, { 1, 1, 1, 1}, {1, 0}},	// 6
			{{ 0.5f, -0.5f,  0.5f}, { 1, 1, 1, 1}, {1, 1}},	// 7
		};

		const ui32 cubeIndices[] = {
			0,1,2, 0,2,3,	// 前
			4,5,6, 4,7,6,	// 後
			4,5,1, 4,1,0,	// 左
			3,2,6, 3,6,7,	// 右
			1,5,6, 1,6,2,	// 上
			4,0,3, 4,3,7,	// 下
		};

		auto vb = _device.CreateVertexBuffer({ cubeVertices, std::size(cubeVertices), sizeof(Vertex) });
		auto ib = _device.CreateIndexBuffer({ cubeIndices, std::size(cubeIndices) });

		return ecs::Mesh{ vb, ib };
	}
}