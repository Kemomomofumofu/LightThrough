#pragma once
/**
 * @file GroundContact.h
 * @brief 地面接触コンポーネント
 */

 // ---------- インクルード ---------- //
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {
	/**
	 * @brief 地面接触コンポーネント
	 */
	struct GroundContact {
		bool isGrounded = false;	// 地面に接触しているか
		float groundNormalY = 0.0f;	// 接触面法線のY成分最大値
	};
}


ECS_REFLECT_BEGIN(ecs::GroundContact)
ECS_REFLECT_FIELD_INSPECT_ONLY(isGrounded),
ECS_REFLECT_FIELD_INSPECT_ONLY(groundNormalY)
ECS_REFLECT_END()