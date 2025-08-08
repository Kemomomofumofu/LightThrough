#pragma once
/**
 * @file InputListener.h
 * @brief 入力システムの通知を受け取る
 * @author Arima Keita
 * @date 2025-07-30
 */

// ---------- インクルード ---------- // 
#include <DX3D/Math/Point.h>

namespace input {
	/**
	 * @brief インプットリスナー
	 *
	 * 入力システムの通知を受け取るために継承するクラス
	 */
	class InputListener {
	public:
		InputListener() {}
		~InputListener() {}

		// キーボードの入力
		// [ToDo] Triggerも追加するべき。
		virtual void OnKeyDown(int _key) = 0;
		virtual void OnKeyUp(int _key) = 0;

		// マウス入力
		virtual void OnMouseMove(const dx3d::Point& _deltaMausePos) = 0;
	};
}