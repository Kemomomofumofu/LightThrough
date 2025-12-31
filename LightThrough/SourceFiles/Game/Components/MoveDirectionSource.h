#pragma once


// ---------- インクルード ---------- // 
#include <string>
#include <DirectXMath.h>
#include <Game/ECS/Entity.h>
#include <Game/Serialization/ComponentReflection.h>

#include <Game/ECS/ECSUtils.h>

namespace ecs {
	struct MoveDirectionSource {
		std::string targetName;


		Entity target{};	// 参照先エンティティ
	};
}


ECS_REFLECT_BEGIN(ecs::MoveDirectionSource)
ECS_REFLECT_FIELD(targetName)
ECS_REFLECT_END()


