#pragma once
/**
 * @file InputSystem.h
 * @brief 入力を管理するクラス(シングルトン)
 * @author Arima Keita
 * @date 2025-07-30
 */

// ---------- インクルード ---------- // 
#include <unordered_set>
#include <InputSystem/InputListener.h>
#include <DX3D/Math/Point.h>

namespace input {

	/**
	 * @brief インプットシステム
	 *
	 * 入力を管理するクラス。
	 * [ToDo] 現状は工数削減のためにシングルトンでの実装。
	 * 将来的にはもっといい形にしたいがひとまず妥協する。
	 * パフォーマンス的には変化のあったキーだけ通知する形にしたほうが良い。まぁ追々ですね...
	 */
	class InputSystem
	{
	public:
		void AddListener(InputListener* _listener);
		void RemoveListener(InputListener* _listener);
		void Update();

		static InputSystem& Get();

	private:
		InputSystem() = default;
		~InputSystem() = default;
		// コピームーブ禁止
		InputSystem(const InputSystem&) = delete;
		InputSystem& operator=(const InputSystem&) = delete;

		std::unordered_set<InputListener*> listeners_;	// 通知される方々
		unsigned char keys_state_[256] = {};					// キーステート
		unsigned char old_keys_state_[256] = {};					// 前回のキーステート
		dx3d::Point old_mouse_pos_;
		bool first_time_ = true;
	};
}

