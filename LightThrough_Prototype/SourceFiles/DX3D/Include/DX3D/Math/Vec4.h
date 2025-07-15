#pragma once
/**
 * @file Vec4.h
 * @brief ベクターフォーー
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Core/Core.h>

/**
 * @brief Vector4クラス
 *
 * floatを4つ保持するクラス.
 */
namespace dx3d {
	class Vec4 {
	public:
		Vec4() = default;
		Vec4(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) {}
	public:
		f32 x{}, y{}, z{}, w{};
	};
}