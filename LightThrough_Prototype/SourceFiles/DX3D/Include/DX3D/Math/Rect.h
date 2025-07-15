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
		Rect(i32 _width, i32 _height) : left(0), top(0), width(_width), height(_height){}
		Rect(i32 _left, i32 _top, i32 _width, i32 _height) : left(_left), top(_top), width(_width), height(_height){}
	public:
		i32 left{}, top{}, width{}, height{};
	};

}