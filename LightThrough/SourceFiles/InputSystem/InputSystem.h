#pragma once
/**
 * @file InputSystem.h
 * @brief 入力を管理するクラス(シングルトン)
 * @author Arima Keita
 * @date 2025-07-30
 */

// ---------- インクルード ---------- // 
#include <Windows.h>
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
		void Init(HWND _hwnd);
		void Update();
		void AddListener(InputListener* _listener);
		void RemoveListener(InputListener* _listener);

		static InputSystem& Get();
		bool IsMouseLocked() const;

		void LockMouse(bool _lock);
		void SetFocus(bool _focused);
	private:
		InputSystem() = default;
		~InputSystem() = default;
		// コピームーブ禁止
		InputSystem(const InputSystem&) = delete;
		InputSystem& operator=(const InputSystem&) = delete;

	private:
		std::unordered_set<InputListener*> listeners_;	// 通知される方々
		unsigned char keys_state_[256] = {};					// キーステート
		unsigned char old_keys_state_[256] = {};					// 前回のキーステート
		dx3d::Point old_mouse_pos_;
		bool first_time_ = true;
		bool mouse_locked_ = false; // マウスロックされているか
		bool focused_ = true;		// ウィンドウにフォーカスが当たっているか

		HWND hwnd_{};	// ウィンドウハンドル
	};
}

