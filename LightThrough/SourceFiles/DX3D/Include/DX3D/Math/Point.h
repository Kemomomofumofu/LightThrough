#pragma once

/**
 * @file Point.h
 * @brief
 * @author Arima Keita
 * @date 2025-07-31
 */


 // ---------- インクルード ---------- // 
#include <DX3D/Core/Core.h>

namespace dx3d {

	/**
	 * @brief 点クラス
	 *
	 * 平面上での座標を保持するクラス
	 */
	class Point {
	public:
		Point() : x(0), y(0) {}
		Point(int _x, int _y) : x(static_cast<f32>(_x)), y(static_cast<f32>(_y)) {}
		Point(f32 _x, f32 _y) : x(_x), y(_y) {}
		Point(const Point& _point) : x(_point.x), y(_point.y) {}
		~Point() {}

	public:
		f32 x = 0, y = 0;
	};
}
