#pragma once

/**
 * @file ObjectRoot.h
 * @brief エディター上で複数Entityをまとめて管理するための情報
 */

// ---------- インクルード ---------- // 
#include <string>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {
	struct ObjectRoot
	{
		std::string name = "None"; // 表示名
	};
}

ECS_REFLECT_BEGIN(ecs::ObjectRoot)
ECS_REFLECT_FIELD(name)
ECS_REFLECT_END()