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
		Point(int _x, int _y) : x(static_cast<float>(_x)), y(static_cast<float>(_y)) {}
		Point(float _x, float _y) : x(_x), y(_y) {}
		Point(const Point& _point) : x(_point.x), y(_point.y) {}
		~Point() {}

	public:
		float x = 0, y = 0;
	};
}
