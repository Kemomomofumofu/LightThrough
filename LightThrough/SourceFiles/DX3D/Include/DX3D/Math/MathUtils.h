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
		 * @param _a ベクトルA
		 * @param _b ベクトルB
		 * @return 差
		 */
		inline XMFLOAT3 Sub(const XMFLOAT3& _a, const XMFLOAT3& _b)
		{
			return { _a.x - _b.x, _a.y - _b.y, _a.z - _b.z };
		}

		inline XMFLOAT3 Add(const XMFLOAT3& _a, const XMFLOAT3& _b)
		{
			return { _a.x + _b.x, _a.y + _b.y, _a.z + _b.z };
		}



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
		 * @brief 外積
		 * @param _a ベクトルA
		 * @param _b ベクトルB
		 * @return 外積
		 */
		inline XMFLOAT3 Cross(const XMFLOAT3& _a, const XMFLOAT3& _b)
		{
			return {
				_a.y * _b.z - _a.z * _b.y,
				_a.z * _b.x - _a.x * _b.z,
				_a.x * _b.y - _a.y * _b.x
			};
		}


		/**
		 * @brief ベクトルの長さ
		 * @param _v ベクトル
		 * @return 長さ
		 */
		inline float LengthSq(const XMFLOAT3& _v)
		{
			return Dot(_v, _v);
		}

		/**
		 * @brief ベクトルの長さ
		 * @param _v ベクトル
		 * @return 長さ
		 */
		inline float Length(const XMFLOAT3& _v)
		{
			return std::sqrt(LengthSq(_v));
		}

	} // namespace math
} // namespace dx3d

namespace math = dx3d::math;