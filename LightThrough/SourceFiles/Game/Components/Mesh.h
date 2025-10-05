#pragma once
/**
 * @file Mesh.h
 * @brief VertexBufferやIndexBufferを保持するクラス
 * [ToDo] SkeletalMeshなどの情報を持たせるようにする
 * @author Arima Keita
 * @date 2025-09-03
 */

// ---------- インクルード ---------- // 
#include <memory>
#include <DX3D/Graphics/Buffers/VertexBuffer.h>
#include <DX3D/Graphics/Buffers/IndexBuffer.h>

namespace ecs {
	struct Mesh {
		std::shared_ptr<dx3d::VertexBuffer> vb;
		std::shared_ptr<dx3d::IndexBuffer> ib;
		uint32_t indexCount{};
		uint32_t materialId{};	// 仮置き
	};
}
