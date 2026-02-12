#pragma once
/**
 * @file SpriteRenderer.h
 * @brief SpriteのHandleを持つコンポーネント
 */

 // ---------- インクルード ---------- // 
#include <memory>
#include <DX3D/Graphics/Textures/TextureHandle.h>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {
	struct SpriteRenderer {
		std::string spriteName = "Assets/Textures/hogehoge.png"; // スプライト名(読み込み用)

		DirectX::XMFLOAT2 size = { 64.0f, 64.0f }; // 描画サイズ(幅, 高さ)
		DirectX::XMFLOAT2 pivot = { 0.5f, 0.5f }; // ピボット(0..1, 0..1) デフォルトは中心
		DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 色(乗算用)

		int layer = 0; // 描画レイヤー（整数、値が大きいほど後ろに描画される）

		::dx3d::TextureHandle handle{};	// ランタイム用
	};
}

ECS_REFLECT_BEGIN(ecs::SpriteRenderer)
ECS_REFLECT_FIELD(spriteName),
ECS_REFLECT_FIELD(size),
ECS_REFLECT_FIELD(pivot),
ECS_REFLECT_FIELD(color),
ECS_REFLECT_FIELD(layer)
ECS_REFLECT_END()