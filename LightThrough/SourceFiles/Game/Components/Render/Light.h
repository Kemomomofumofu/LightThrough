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

	constexpr int MAX_LIGHTS = 64;

	struct LightViewProj
	{
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
	};

	enum class LightType : uint32_t
	{
		Directional = 0,
		Spot,
		//Point,
	};

	//! @brief ライト共通データ
	struct LightCommon
	{
		DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f };	// 色
		float intensity = 100.0f;									// 乗算係数
		bool enabled = true;									// 有効フラグ
		uint32_t _pad0[3]{};
	};



	//! @brief 平行光源
	struct DirectionalLight
	{
		// Transformを使用
	};


	//! @brief スポットライト
	struct SpotLight
	{
		float range = 100.0f;	// 到達距離
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
	struct LightCPU
	{
		/* type */
		// 0: Directional
		// 1: Spot
		DirectX::XMFLOAT4 pos_type{}; // xyz = pos, w = type
		DirectX::XMFLOAT4 dir_range{}; // xyz = dir, w = range
		DirectX::XMFLOAT4 color{};
		DirectX::XMFLOAT4 spotAngles_shadowIndex{}; // x = innerCos, y = outerCos, z = shadowMapIndex ,w 未使用
	};

	struct CBLight
	{
		int lightCount; uint32_t _pad0[3];
		LightCPU lights[MAX_LIGHTS];
		DirectX::XMFLOAT4X4 lightViewProj[MAX_LIGHTS]; // 各ライトのビュー射影行列
	};

	struct CBLightMatrix
	{
		DirectX::XMFLOAT4X4 lightViewProj;
	};

	static_assert(sizeof(LightCPU) == 64, "LightCPUのサイズが不正(4 * 16 bytes)");


	/**
	 * @brief LightCPU構築
	 * @param _tf transformComponent
	 * @param _common lightCommonComponent
	 * @param _spot spotLightComponentのポインタ(スポットライトでない場合はnullptr)
	 * @return 構築されたLightCPU
	 */
	inline LightCPU BuildLightCPU(const Transform* _tf, const LightCommon* _common, const SpotLight* _spot)
	{
		using namespace DirectX;
		LightCPU L{};

		const XMFLOAT3& pos = _tf->GetWorldPosition();
		const XMFLOAT3& fwd = _tf->GetWorldForward();
		L.pos_type = { pos.x, pos.y, pos.z, 0.0f };
		L.dir_range = { fwd.x, fwd.y, fwd.z, 0.0f };
		L.color = {
			_common->color.x * _common->intensity,
			_common->color.y * _common->intensity,
			_common->color.z * _common->intensity,
			1.0f
		};
		L.spotAngles_shadowIndex = { 0.0f, 0.0f, 0.0f, 0.0f };

		if (_spot) {
			L.pos_type.w = 1.0f; // Spot
			L.dir_range.w = _spot->range;
			L.spotAngles_shadowIndex.x = _spot->innerCos;
			L.spotAngles_shadowIndex.y = _spot->outerCos;
		}

		return L;
	}

	inline LightViewProj BuildLightViewProj(const Transform* _tf, const SpotLight* _spot, float _nearZ = 0.1f)
	{
		using namespace DirectX;
		
		// Transformの現在の方向ベクトルを取得
		XMVECTOR f = _tf->GetWorldForwardV();
		XMVECTOR u = _tf->GetWorldUpV();
		
		// 前方向と上方向がほぼ平行な場合は別の上方向を使用
		float dotFU = XMVectorGetX(XMVector3Dot(f, u));
		if (fabsf(dotFU) > 0.98f) {
			// 別軸を試す
			XMVECTOR alt = XMVectorSet(0, 0, 1, 0);
			if (fabsf(XMVectorGetX(XMVector3Dot(f, alt))) > 0.9f) {
				alt = XMVectorSet(1, 0, 0, 0);
			}
			u = XMVector3Normalize(XMVector3Cross(alt, f));
		}
		else {
			// 直交化: upからforwardへの射影成分を除去
			u = XMVector3Normalize(u - f * XMVectorGetX(XMVector3Dot(f, u)));
		}
		
		// ビュー行列を構築
		XMVECTOR eye = _tf->GetWorldPositionV();
		XMMATRIX view = XMMatrixLookToLH(eye, f, u);

		// 射影行列を構築
		XMMATRIX proj;
		if (_spot) {
			float fovY = _spot->CulcFovYRadians();
			float farZ = (std::max)(1.0f, _spot->range);
			proj = XMMatrixPerspectiveFovLH(fovY, 1.0f, _nearZ, farZ);
		}
		else {
			float orthoSize = 20.0f;
			proj = XMMatrixOrthographicLH(orthoSize * 2.0f, orthoSize * 2.0f, 0.1f, 1000.0f);
		}

		return { view, proj };
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
