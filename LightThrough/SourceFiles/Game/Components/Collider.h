#pragma once
/**
 * @file Collider.h
 * @brief コライダーコンポーネント
 * @author Arima Keita
 * @date 2025-10-03
 */

 // ---------- インクルード ---------- //
#include <Game/Collisions/CollisionUtils.h>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {
	/**
	 * @brief コライダーコンポーネント
	*/
	struct Collider {
		collision::ShapeType type{ collision::ShapeType::Box };

		collision::BoxShape box;
		collision::SphereShape sphere;
		bool isTrigger = false;
		bool isStatic = false;
		bool shapeDirty = true;	// 形状が変更されたか

		// キャッシュ
		collision::ShapeVariant shape{};
		collision::WorldSphere worldSphere{};
		collision::WorldOBB worldOBB{};
		float broadPhaseRadius = 0.0f; // ブロードフェーズ用の半径
	};
}


ECS_REFLECT_BEGIN(ecs::Collider)
ECS_REFLECT_FIELD(type),
ECS_REFLECT_FIELD(box),
ECS_REFLECT_FIELD(sphere),
ECS_REFLECT_FIELD(isTrigger),
ECS_REFLECT_FIELD(isStatic)
ECS_REFLECT_END()