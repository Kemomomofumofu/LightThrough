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
		DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f };	// 色
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

		//! @brief Fov(Y)ラジアンの計算
		inline float CulcFovYRadians() const noexcept {
			float oc = std::clamp(outerCos, -1.0f, 1.0f);
			return 2.0f * acosf(oc);
		}
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



	inline LightCPU BuildLightCPU(const Transform& _tf, const LightCommon& _common, const SpotLight* _spot)
	{
		using namespace DirectX;
		LightCPU L{};

		const XMFLOAT3& fwd = _tf.GetForward();
		L.pos_type = { _tf.position.x, _tf.position.y, _tf.position.z, 0.0f };
		L.dir_range = { fwd.x, fwd.y, fwd.z, 0.0f };
		L.color = {
			_common.color.x * _common.intensity,
			_common.color.y * _common.intensity,
			_common.color.z * _common.intensity,
			1.0f
		};
		L.spotAngles = { 0.0f, 0.0f, 0.0f, 0.0f };

		if (_spot) {
			L.pos_type.w = 1.0f; // Spot
			L.dir_range.w = _spot->range;
			L.spotAngles.x = _spot->innerCos;
			L.spotAngles.y = _spot->outerCos;
		}

		return L;
	}

	inline DirectX::XMMATRIX BuildLightViewProj(const Transform& _tf, const SpotLight* _spot, float _nearZ = 0.1f)
	{
		using namespace DirectX;
		const XMMATRIX V = _tf.MakeLookToLH();
		if (_spot) {
			const float fovY = _spot->CulcFovYRadians();
			const float farZ = (std::max)(0.0f, _spot->range);
			const XMMATRIX P = XMMatrixPerspectiveFovLH(fovY, 1.0f, _nearZ, farZ);

			return XMMatrixMultiply(V, P);
		}
		else {
			const float orthoSize = 20.0f;
			const XMMATRIX P = XMMatrixOrthographicLH(orthoSize * 2.0f, orthoSize * 2.0f, 0.1f, 1000.0f);

			return XMMatrixMultiply(V, P);
		}
	}
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
