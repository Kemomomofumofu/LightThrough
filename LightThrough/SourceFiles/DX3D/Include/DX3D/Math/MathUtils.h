#pragma once
/**
 * @file MathUtils.h
 * @brief 数学関数のヘルパー
 */

 // ---------- インクルード ---------- //
#include <DirectXMath.h>
#include <cmath>

namespace dx3d {
	namespace math {
		using namespace DirectX;

		/**
		 * @brief 減算
		 */
		[[nodiscard]] inline XMFLOAT3 Sub(const XMFLOAT3& _a, const XMFLOAT3& _b) noexcept
		{
			return { _a.x - _b.x, _a.y - _b.y, _a.z - _b.z };
		}

		/**
		 * @brief 加算
		 */
		[[nodiscard]] inline XMFLOAT3 Add(const XMFLOAT3& _a, const XMFLOAT3& _b) noexcept
		{
			return { _a.x + _b.x, _a.y + _b.y, _a.z + _b.z };
		}

		/**
		 * @brief スカラー倍
		 */
		[[nodiscard]] inline XMFLOAT3 Scale(const XMFLOAT3& _v, float _s) noexcept
		{
			return { _v.x * _s, _v.y * _s, _v.z * _s };
		}

		/**
		 * @brief ベクトルの反転
		 */
		[[nodiscard]] inline XMFLOAT3 Negate(const XMFLOAT3& _v) noexcept
		{
			return { -_v.x, -_v.y, -_v.z };
		}

		/**
		 * @brief 内積
		 */
		[[nodiscard]] inline float Dot(const XMFLOAT3& _a, const XMFLOAT3& _b) noexcept
		{
			return (_a.x * _b.x) + (_a.y * _b.y) + (_a.z * _b.z);
		}

		/**
		 * @brief 外積
		 */
		[[nodiscard]] inline XMFLOAT3 Cross(const XMFLOAT3& _a, const XMFLOAT3& _b) noexcept
		{
			return {
				_a.y * _b.z - _a.z * _b.y,
				_a.z * _b.x - _a.x * _b.z,
				_a.x * _b.y - _a.y * _b.x
			};
		}


		/**
		 * @brief ベクトルの長さの二乗
		 */
		[[nodiscard]] inline float LengthSq(const XMFLOAT3& _v) noexcept
		{
			return Dot(_v, _v);
		}

		/**
		 * @brief ベクトルの長さ
		 */
		[[nodiscard]] inline float Length(const XMFLOAT3& _v) noexcept
		{
			return std::sqrt(LengthSq(_v));
		}

		/**
		 * @brief ベクトルの正規化
		 * @param _v 正規化するベクトル
		 * @return 正規化されたベクトル
		 */
		[[nodiscard]] inline XMFLOAT3 Normalize(const XMFLOAT3& _v) noexcept
		{
			float len = math::Length(_v);
			if (len < 1e-6f) { return{ 0, 0, 0 }; }
			return { _v.x / len, _v.y / len, _v.z / len };
		}

		/**
		 * @brief 2点間の距離の二乗
		 */
		[[nodiscard]] inline float DistSq(const XMFLOAT3& _a, const XMFLOAT3& _b) noexcept
		{
			auto diff = Sub(_a, _b);
			return Dot(diff, diff);
		}

		/**
		 * @brief 限りなくゼロに近いか
		 * @return true: ゼロに近い, false: そうでない
		 */
		[[nodiscard]] inline bool IsZeroVec(const XMFLOAT3& v) noexcept
		{
			return std::fabs(v.x) + std::fabs(v.y) + std::fabs(v.z) < 1e-8f;
		}
		[[nodiscard]] inline bool IsZeroDisp(float _v) noexcept
		{
			const float eps = 1e-6f;
			return (std::fabs(_v) < eps);
		}


	} // namespace math
} // namespace dx3d

namespace math = dx3d::math;