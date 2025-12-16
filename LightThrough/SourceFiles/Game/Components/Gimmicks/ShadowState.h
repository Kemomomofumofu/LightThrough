#pragma once
/**
 * @file ShadowState.h
 * @brief 影の中にいるかを保持する
 */

 // ---------- インクルード ---------- //
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {
	struct ShadowState
	{
		bool IsInShadow = false;
	};
}

ECS_REFLECT_BEGIN(ecs::ShadowState)
ECS_REFLECT_END()