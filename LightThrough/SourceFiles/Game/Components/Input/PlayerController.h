#pragma once
/**
 * @file PlayerController.h
 * @brief プレイヤー操作に必要なデータ
 */

// ---------- インクルード ---------- // 
#include <Game/Serialization/ComponentReflection.h>


namespace ecs {
	struct PlayerController {
		float moveSpeed = 5.0f; // 移動速度
		float jumpForce = 5.0f; // ジャンプ力
	};
}

ECS_REFLECT_BEGIN(ecs::PlayerController)
ECS_REFLECT_FIELD(moveSpeed),
ECS_REFLECT_FIELD(jumpForce)
ECS_REFLECT_END()