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
		float GetWheelDelta() const;

		void OnRawInput(LPARAM _lParam);
		// RawInputの有効/無効
		void EnableRawMouse(bool _enable);
		bool IsRawMouseEnabled() const;
		
		// マウスフォーカス関連
		void LockMouse(bool _lock);
		void SetFocus(bool _focused);
		// 入力の有効/無効
		void SetInputEnabled(bool _enable);
		bool IsInputEnabled() const;

	private:
		InputSystem() = default;
		~InputSystem() = default;
		// コピームーブ禁止
		InputSystem(const InputSystem&) = delete;
		InputSystem& operator=(const InputSystem&) = delete;

		void RegisterRawMouse();	// RawInputの登録
		void ClearFrameMouse();		// フレーム開始処理

	private:
		unsigned char keys_state_[256] = {};			// キーステート
		unsigned char old_keys_state_[256] = {};		// 前回のキーステート
		dx3d::Point mouse_delta_{};						// マウスの移動量
		float wheel_delta_{};							// ホイールの移動量

		bool first_time_ = true;
		bool input_enabled_ = false; // 入力が有効か
		bool mouse_locked_ = false; // マウスロックされているか
		bool focused_ = true;		// ウィンドウにフォーカスが当たっているか

		// RawInput
		std::vector<uint8_t> raw_buffer_{};	// RawInputのバッファ
		dx3d::Point raw_mouse_accum_{};	// マウス入力の累積値

		bool raw_mouse_registered_ = false;
		bool use_raw_mouse_ = true; // RawInputをを使うか

		HWND hwnd_{};	// ウィンドウハンドル
	};
}

