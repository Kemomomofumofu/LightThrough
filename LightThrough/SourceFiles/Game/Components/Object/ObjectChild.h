#pragma once
/**
 * @file ObjectChild.h
 * @brief エディター上で複数Entityをまとめて管理するための情報
 */

 // ---------- インクルード ---------- // 
#include <string>
#include <Game/Serialization/ComponentReflection.h>

#include <Game/ECS/Entity.h>

namespace ecs {
	struct ObjectChild
	{
		// シリアライズ用
		std::string rootName; // 親の名前

		// ランタイム用
		Entity root; // 親
	};
}

ECS_REFLECT_BEGIN(ecs::ObjectChild)
ECS_REFLECT_FIELD(rootName)
ECS_REFLECT_END()