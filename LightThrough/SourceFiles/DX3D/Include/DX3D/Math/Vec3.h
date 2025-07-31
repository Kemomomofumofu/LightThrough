#pragma once
/**
 * @file Vec3.h
 * @brief ベクタースリー
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Core/Core.h>

/**
 * @brief Vector3クラス
 *
 * floatを3つ保持するクラス.
 */
namespace dx3d {
	class Vec3 {
	public:
		Vec3() = default;
		Vec3(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z){}
	public:
		f32 x{}, y{}, z{};
	};
}