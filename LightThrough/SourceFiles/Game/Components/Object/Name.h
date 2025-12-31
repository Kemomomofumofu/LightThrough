#pragma once

/**
 * @file Name.h
 * @brief Entityに文字での識別を可能にするための情報
 */

// ---------- インクルード ---------- // 
#include <string>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {
	struct Name
	{
		std::string value; // 名前
	};
}

ECS_REFLECT_BEGIN(ecs::Name)
ECS_REFLECT_FIELD(value)
ECS_REFLECT_END()