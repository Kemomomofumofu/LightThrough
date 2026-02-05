#pragma once
/**
 * @file Sprite.h
 * @brief SpriteのHandleを持つコンポーネント
 */

 // ---------- インクルード ---------- // 
#include <memory>
#include <DX3D/Graphics/Textures/TextureHandle.h>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {
	struct Sprite {
		std::string spriteName = "Assets/Texture/hogehoge.png"; // スプライト名(読み込み用)
		::dx3d::TextureHandle handle{};	// ランタイム用
	};
}

ECS_REFLECT_BEGIN(ecs::Sprite)
ECS_REFLECT_FIELD(spriteName)
ECS_REFLECT_END()