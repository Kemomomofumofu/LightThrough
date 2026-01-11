#pragma once
/**
 * @file MeshRenderer.h
 * @brief MeshのHandleを持つコンポーネント
 * @author Arima Keita
 * @date 2025-09-03
 */

 // ---------- インクルード ---------- // 
#include <memory>
#include <DX3D/Graphics/Meshes/MeshHandle.h>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {
	struct MeshRenderer {
		::dx3d::MeshHandle handle{};
		uint32_t materialId{};	// [ToDo] 仮置き 将来的にはMaterialHandleにする
	};
}

ECS_REFLECT_BEGIN(ecs::MeshRenderer)
ECS_REFLECT_FIELD(handle),
ECS_REFLECT_FIELD(materialId)
ECS_REFLECT_END()