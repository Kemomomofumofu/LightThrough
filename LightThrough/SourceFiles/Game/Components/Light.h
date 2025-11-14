#pragma once

/**
 * @file Light.h
 * @brief Brief description of this file.
 */

 // ---------- インクルード ---------- // 
#include <cstdint>
#include <DirectXMath.h>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {

	constexpr int MAX_LIGHTS = 16;

	enum class LightType : uint32_t {
		Directional = 0,
		Spot,
		//Point,
	};

	//! ライト共通データ
	struct LightCommon {
		DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f};	// 色
		float intensity = 1.0f;									// 乗算係数
		bool enabled = true;									// 有効フラグ
		uint32_t _pad0[3];
	};

	//! 平行光源
	struct DirectionalLight {
		// Transformを使用
	};


	//! @brief スポットライト
	struct SpotLight {
		float range = 10.0f;	// 到達距離
		float innerCos = 0.9f;	// 内側コサイン
		float outerCos = 0.8f;	// 外側コサイン
		uint32_t _pad0{};
	};

	// GPUに送る際に使う構造体
	struct LightCPU {
		/* type */
		// 0: Directional
		// 1: Spot
		DirectX::XMFLOAT4 pos_type{}; // xyz = pos, w = type
		DirectX::XMFLOAT4 dir_range{}; // xyz = dir, w = range
		DirectX::XMFLOAT4 color{};
		DirectX::XMFLOAT4 spotAngles{}; // x = innerCos, y = outerCos, z,w 未使用
	};

	struct CBLight {
		int lightCount; uint32_t _pad0[3];
		LightCPU lights[MAX_LIGHTS];
	};

	struct CBLightMatrix {
		DirectX::XMMATRIX lightViewProj;
	};

	static_assert(sizeof(LightCPU) == 64, "LightCPUのサイズが不正(4 * 16 bytes)");
}

// ---------- コンポーネント反映定義 ---------- //
// ライト共通データ
ECS_REFLECT_BEGIN(ecs::LightCommon)
ECS_REFLECT_FIELD(color),
ECS_REFLECT_FIELD(intensity),
ECS_REFLECT_FIELD(enabled)
ECS_REFLECT_END()
// 平行光源
ECS_REFLECT_BEGIN(ecs::DirectionalLight)
ECS_REFLECT_END()
// スポットライト
ECS_REFLECT_BEGIN(ecs::SpotLight)
ECS_REFLECT_FIELD(range),
ECS_REFLECT_FIELD(innerCos),
ECS_REFLECT_FIELD(outerCos)
ECS_REFLECT_END()
