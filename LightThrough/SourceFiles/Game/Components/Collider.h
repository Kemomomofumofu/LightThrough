#pragma once
/**
 * @file Collider.h
 * @brief コライダーコンポーネント
 * @author Arima Keita
 * @date 2025-10-03
 */

 // ---------- インクルード ---------- //
#include <Game/Collisions/CollisionUtils.h>

namespace ecs {
	/**
	 * @brief コライダーコンポーネント
	*/
	struct Collider {
		collision::ShapeType type{collision::ShapeType::Box};
		collision::ShapeVariant shape{};
		bool isTrigger = false;
		bool isStatic = false;
		bool shapeDirty = true;	// 形状が変更されたか

		// キャッシュ
		collision::WorldSphere worldSphere{};
		collision::WorldOBB worldOBB{};

		float broadPhaseRadius = 0.0f; // ブロードフェーズ用の半径
	};
}