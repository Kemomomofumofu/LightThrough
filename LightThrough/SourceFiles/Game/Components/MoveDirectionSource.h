#pragma once


// ---------- インクルード ---------- // 
#include <DirectXMath.h>
#include <Game/ECS/Entity.h>
#include <Game/Serialization/ComponentReflection.h>

#include <Game/ECS/ECSUtils.h>

namespace ecs {
	struct MoveDirectionSource {
		Entity target{};	// 参照先エンティティ
	};
}


ECS_REFLECT_BEGIN(ecs::MoveDirectionSource)
ECS_REFLECT_FIELD(target)
ECS_REFLECT_END()


