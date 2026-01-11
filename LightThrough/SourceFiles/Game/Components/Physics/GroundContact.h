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
		// Entity groundEntity{};	// 接触している地面のEntity memo: 動く床が出るなら使う
		// float distance = 0.0f;		// 接触距離 
	};
}

ECS_REFLECT_BEGIN(ecs::GroundContact)
ECS_REFLECT_FIELD(isGrounded)
ECS_REFLECT_END()