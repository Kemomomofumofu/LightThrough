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
	 * もともとオブザーバーパターンで入力のあったキーを通知する形だったが、
	 * ECSとの親和性を考慮して、押されているキーの状態を問い合わせる形に変更した。
	 * 一応残しておく
	 */
	class InputSystem
	{
	public:
		static InputSystem& Get();

		void Init(HWND _hwnd);
		void Update();

		bool IsMouseLocked() const;		// マウスがロックされているか
		bool IsKeyDown(int _key) const;	// キーが押されているか
		bool IsKeyUp(int _key) const;	// キーが離されているか
		bool IsKeyTrigger(int _key) const;	// キーが押された瞬間か
		bool IsKeyRelease(int _key) const;	// キーが離された瞬間か
		dx3d::Point GetMouseDelta();

		//void AddListener(InputListener* _listener);
		//void RemoveListener(InputListener* _listener);

		void LockMouse(bool _lock);		// マウスをロックするか
		void SetFocus(bool _focused);	// フォーカス設定

	private:
		InputSystem() = default;
		~InputSystem() = default;
		// コピームーブ禁止
		InputSystem(const InputSystem&) = delete;
		InputSystem& operator=(const InputSystem&) = delete;

	private:
		//std::unordered_set<InputListener*> listeners_;	// 通知される方々
		unsigned char keys_state_[256] = {};			// キーステート
		unsigned char old_keys_state_[256] = {};		// 前回のキーステート
		dx3d::Point mouse_delta_{};						// マウスの移動量
		bool first_time_ = true;
		bool mouse_locked_ = false; // マウスロックされているか
		bool focused_ = true;		// ウィンドウにフォーカスが当たっているか

		HWND hwnd_{};	// ウィンドウハンドル
	};
}

