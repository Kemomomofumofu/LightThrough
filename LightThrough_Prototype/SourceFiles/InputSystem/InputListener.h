#pragma once
/**
 * @file InputListener.h
 * @brief 入力システムの通知を受け取る
 * @author Arima Keita
 * @date 2025-07-30
 */

namespace LightThrough {
	/**
	 * @brief インプットリスナー
	 *
	 * 入力システムの通知を受け取るために継承するクラス
	 */
	class InputListener {
	public:
		InputListener() {}
		~InputListener() {}

		virtual void OnKeyDown(int _key) = 0;
		virtual void OnKeyUp(int _key) = 0;
	}
}