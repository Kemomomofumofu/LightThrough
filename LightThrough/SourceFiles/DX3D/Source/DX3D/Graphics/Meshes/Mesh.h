#pragma once
/**
 * @file Mesh.h
 * @brief VertexBufferやIndexBufferを保持するクラス
 * @author Arima Keita
 * @date 2025-09-03
 */

 // ---------- インクルード ---------- //
#include <DX3D/Graphics/Buffers/IndexBuffer.h>
#include <DX3D/Graphics/Buffers/VertexBuffer.h>

namespace dx3d {
	struct Mesh {
		std::shared_ptr<VertexBuffer> vb;
		std::shared_ptr<IndexBuffer> ib;
		uint32_t indexCount{};
	};
}