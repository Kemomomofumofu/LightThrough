#pragma once
/**
 * @file Lifetime.h
 * @brief エンティティの寿命を管理するコンポーネント
 */

// ---------- インクルード ---------- //
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {
	/**
	 * @brief エンティティの寿命を管理するコンポーネント
	 */
	struct Lifetime {
		float lifetime = 0.0f;			// 残り寿命(秒)
		bool infiniteLifetime = false;	// 無限寿命フラグ
	};
}