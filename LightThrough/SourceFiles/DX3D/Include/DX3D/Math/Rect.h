#pragma once

/**
 * @file Rect.h
 * @brief 矩形を定義するヘッダ
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Core/Core.h>


/**
 * @brief 矩形を定義するクラス
 *
 * 上下左右のサイズを保持する
 */
namespace dx3d {
	class Rect {
	public:
		Rect() = default;
		Rect(int32_t _width, int32_t _height) : left(0), top(0), width(_width), height(_height){}
		Rect(int32_t _left, int32_t _top, int32_t _width, int32_t _height) : left(_left), top(_top), width(_width), height(_height){}
	public:
		int32_t left{}, top{}, width{}, height{};
	};

}