#pragma once
/**
 * @file CollisionOBB.h
 * @brief 当たり判定のヘルパー的関数
 * @author Arima Keita
 * @date 2025-10-03
 */

 // ---------- インクルード ---------- //
#include <array>
#include <cmath>
#include <algorithm>
#include <optional>
#include <DirectXMath.h>


namespace collision {
	using namespace DirectX;

	/**
	 * @brief ワールドのOBB情報
	 */
	struct OBBWorld {
		XMFLOAT3 center{};	// 中心
		XMFLOAT3 axis[3];	// 各軸
		XMFLOAT3 half{};	// 半径
	};

	/**
	 * @brief OBB同士の当たり判定
	 */
	struct ContactResult {
		float penetration{};		// 衝突深度
		XMFLOAT3 normal{};	// AからBへの法線
	};

	/**
	 * @brief 内積
	 * @param _a ベクトルA
	 * @param _b ベクトルB
	 * @return 内積
	 */
	inline float Dot(const XMFLOAT3& _a, const XMFLOAT3& _b)
	{
		return (_a.x * _b.x) + (_a.y * _b.y) + (_a.z * _b.z);
	}

	/**
	 * @brief 減算
	 * @param _a ベクトルA
	 * @param _b ベクトルB
	 * @return 差
	 */
	inline XMFLOAT3 Sub(const XMFLOAT3& _a, const XMFLOAT3& _b)
	{
		return { _a.x - _b.x, _a.y - _b.y, _a.z - _b.z };
	}

	/**
	 * @brief ベクトルの投影長を計算
	 * @param _b OBB
	 * @param _axis 投影する軸
	 * @return 投影長
	 */
	inline float ProjectRadius(const OBBWorld& _b, const XMFLOAT3& _axis)
	{
		return std::fabs(Dot(_b.axis[0], _axis)) * _b.half.x +
			std::fabs(Dot(_b.axis[1], _axis)) * _b.half.y +
			std::fabs(Dot(_b.axis[2], _axis)) * _b.half.z;

	}

	inline XMFLOAT3 Normalize(const XMFLOAT3& _v)
	{
		float len = std::sqrt(Dot(_v, _v));
		if (len < 1e-6f) { return{ 0, 0, 0 }; }

		return { _v.x / len, _v.y / len, _v.z / len };
	}

	// [ToDo] ここを実装する
	inline std::optional<ContactResult> IntersectOBB(const OBBWorld& _a, const OBBWorld& _b);
}