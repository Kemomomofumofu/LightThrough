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

	/**
	 * @brief ベクトルの正規化
	 * @param _v ベクトル
	 * @return 正規化されたベクトル
	 */
	inline XMFLOAT3 Normalize(const XMFLOAT3& _v)
	{
		float len = std::sqrt(Dot(_v, _v));
		// 長さがほとんど0なら0ベクトルを返す
		if (len < 1e-6f) { return{ 0, 0, 0 }; }

		return { _v.x / len, _v.y / len, _v.z / len };
	}

	/**
	 * @brief OBB同士の当たり判定
	 * @param _a OBB A
	 * @param _b OBB B
	 * @return 衝突している: 衝突情報, していない: std::nullopt
	 */
	inline std::optional<ContactResult> IntersectOBB(const OBBWorld& _a, const OBBWorld& _b)
	{
		constexpr float EPS = 1e-5f; // 浮動小数点誤差の許容範囲
		XMFLOAT3 T = Sub(_b.center, _a.center); // AからBへのベクトル

		XMFLOAT3 testAxes[15]; // テストする軸
		int axisCount = 0;
		for (int i = 0; i < 3; ++i) {
			testAxes[axisCount++] = _a.axis[i];
		}
		for (int i = 0; i < 3; ++i) {
			testAxes[axisCount++] = _b.axis[i];
		}

		// 3x3の外積を計算して追加
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				XMFLOAT3 c{
					_a.axis[i].y * _b.axis[j].z - _a.axis[i].z * _b.axis[j].y,
					_a.axis[i].z * _b.axis[j].x - _a.axis[i].x * _b.axis[j].z,
					_a.axis[i].x * _b.axis[j].y - _a.axis[i].y * _b.axis[j].x
				};
				float len2 = Dot(c, c);
				// 誤差範囲より大きければ
				if (len2 > EPS) {
					// 正規化して追加
					testAxes[axisCount++] = Normalize(c);
				}
			}
		}

		// 最小の貫通深度とその軸
		float minPenetration = std::numeric_limits<float>::max();
		XMFLOAT3 bestAxis{};

		// 各軸でテスト
		for (int i = 0; i < axisCount; ++i) {
			const auto& axis = testAxes[i];

			if (std::fabs(axis.x) + std::fabs(axis.y) + std::fabs(axis.z) < EPS) { continue; }	// ゼロベクトルはスキップ

			float rA =
				std::fabs(Dot(_a.axis[0], axis)) * _a.half.x +
				std::fabs(Dot(_a.axis[1], axis)) * _a.half.y +
				std::fabs(Dot(_a.axis[2], axis)) * _a.half.z;

			float rB = ProjectRadius(_b, axis);

			// 投射軸上での中心距離
			float dist = std::fabs(Dot(T, axis));
			float overlap = rA + rB - dist;

			// 分離している
			if (overlap < 0.0f) {
				return std::nullopt;
			}

			// 最小の貫通深度を更新
			if (overlap < minPenetration) {
				minPenetration = overlap;
				bestAxis = axis;

				// 内積が負なら
				if (Dot(bestAxis, T) < 0.0f) {
					// 中心方向に合わせて法線を反転
					bestAxis.x *= -1.0f;
					bestAxis.y *= -1.0f;
					bestAxis.z *= -1.0f;
				}
			}
		}

		return ContactResult{ minPenetration, bestAxis };
	}
}