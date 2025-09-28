/**
 * @file InputSystem.cpp
 * @brief 入力を管理するクラス
 * @author Arima Keita
 * @date 2025-07-30
 */

 // ---------- インクルード ---------- // 
#include <Game/InputSystem/InputSystem.h>
#include <Windows.h>
#include <hidusage.h>


namespace input {
	/**
	 * @brief ゲッター
	 * @return インプットシステムの参照
	 */
	InputSystem& InputSystem::Get() {
		static InputSystem system;
		return system;
	}

	/**
	 * @brief 初期化
	 * @param _hwnd ウィンドウハンドル
	 */
	void InputSystem::Init(HWND _hwnd)
	{
		hwnd_ = _hwnd;

		// RawInputの登録
		RegisterRawMouse();

		// マウスの初期状態
		LockMouse(true);
		input_enabled_ = true;
	}

	void InputSystem::Update()
	{
		if (!focused_ || !input_enabled_) {
			mouse_delta_ = {};
			return;
		}

		// ---------- キーボード ---------- //
		::memcpy(old_keys_state_, keys_state_, sizeof(keys_state_));
		::GetKeyboardState(keys_state_);

		// ---------- マウス ---------- //

		// RawInputが有効なら 
		if (use_raw_mouse_) {
			ClearFrameMouse();	// フレーム開始処理
		}
		// RawInputが無効なら
		else {
			POINT current{};
			::GetCursorPos(&current);

			if (mouse_locked_) {
				// ウィンドウの中心を取得
				RECT clientRect{};
				::GetClientRect(hwnd_, &clientRect);
				POINT center{
					(clientRect.right - clientRect.left) / 2,
					(clientRect.bottom - clientRect.top) / 2
				};
				::ClientToScreen(hwnd_, &center);

				// 相対移動量
				mouse_delta_.x = current.x - center.x;
				mouse_delta_.y = current.y - center.y;

				// 中央に戻す
				::SetCursorPos(center.x, center.y);
			}
			else {
				// 非ロック時は前フレームとの差分をとる
				static POINT prev{};
				mouse_delta_.x = current.x - prev.x;
				mouse_delta_.y = current.y - prev.y;

				prev = current;
			}
		}
	}


	/**
	 * @brief マウス固定されているか
	 * @return 固定されているならtrue
	 */
	bool InputSystem::IsMouseLocked() const
	{
		return mouse_locked_;
	}


	/**
	 * @brief キーが押されているか
	 * @param _key	問い合わせるキー
	 * @return 押されている: true, そうでない: false
	 */
	bool InputSystem::IsKeyDown(int _key) const
	{
		return keys_state_[_key] & 0x80;
	}

	/**
	 * @brief キーが離されているか
	 * @param _key	問い合わせるキー
	 * @return 離されている: true, そうでない: false
	 *
	 * [ToDo] 実装しつつもIsKeyDownでよくないか...いちおう残しとくけど...
	 */
	bool InputSystem::IsKeyUp(int _key) const
	{
		return !(keys_state_[_key] & 0x80);
	}

	/**
	 * @brief キーが押された瞬間か
	 * @param _key	問い合わせるキー
	 * @return 押された瞬間: true, そうでない: false
	 */
	bool InputSystem::IsKeyTrigger(int _key) const
	{
		return (keys_state_[_key] & 0x80) && !(old_keys_state_[_key] & 0x80);
	}

	/**
	 * @brief キーが離された瞬間か
	 * @param _key	問い合わせるキー
	 * @return 離された瞬間: true, そうでない: false
	 */
	bool InputSystem::IsKeyRelease(int _key) const
	{
		return !(keys_state_[_key] & 0x80) && (old_keys_state_[_key] & 0x80);
	}

	/**
	 * @brief マウスの移動量を取得
	 * @return マウスの移動量
	 */
	dx3d::Point InputSystem::GetMouseDelta()
	{
		return mouse_delta_;
	}

	/**
	 * @brief ホイールの移動量を取得
	 * @return ホイールの移動量
	 */
	float InputSystem::GetWheelDelta() const
	{
		return wheel_delta_;
	}

	void InputSystem::OnRawInput(LPARAM _lParam)
	{
		if (!use_raw_mouse_) { return; }

		UINT size = 0;
		if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(_lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) != 0 || size == 0) { return; }	// サイズ取得失敗

		raw_buffer_.resize(size);
		if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(_lParam), RID_INPUT, raw_buffer_.data(), &size, sizeof(RAWINPUTHEADER)) != size){ return; }	// データ取得失敗

		auto* raw = reinterpret_cast<RAWINPUT*>(raw_buffer_.data());
		if (raw->header.dwType != RIM_TYPEMOUSE) { return; }	// マウス入力でなければ無視

		const auto& mouse = raw->data.mouse;

		// 相対 or 絶対
		if (!(mouse.usFlags & MOUSE_MOVE_ABSOLUTE)) {
			raw_mouse_accum_.x += static_cast<float>(mouse.lLastX);
			raw_mouse_accum_.y += static_cast<float>(mouse.lLastY);
		}
		else {
			// [ToDo] 多分やらない。
			// 絶対座標デバイス(タブレットなど)
		}

		// ホイール
		if (mouse.usButtonFlags & RI_MOUSE_WHEEL) {
			short wheel = static_cast<short>(mouse.usButtonData);
			wheel_delta_ += static_cast<float>(wheel) / static_cast<float>(WHEEL_DELTA);
		}
	}

	/**
	 * @brief RawInputの有効/無効設定
	 * @param _enable 有効にするか
	 */
	void InputSystem::EnableRawMouse(bool _enable)
	{
		if (use_raw_mouse_ == _enable) { return; }

		use_raw_mouse_ = _enable;
		// 有効にするなら
		if (use_raw_mouse_) {
			RegisterRawMouse();
		}
	}

	/**
	 * @brief RawInputが有効か
	 * @return 有効: true, 無効: false
	 */
	bool InputSystem::IsRawMouseEnabled() const
	{
		return use_raw_mouse_;
	}


	/**
	 * @brief マウス固定メソッド
	 * @param _lock 固定するかしないか
	 *
	 * 	[ToDo] マウスカーソルの固定する、しない、をイベント形式にしたいが、
	 *	WinProcのSetFocusで消す、KillFocusで戻す、だと、
	 *	ゲームがメニュー画面でもマウスが消えてしまうので、今後考える。
	 */
	void InputSystem::LockMouse(bool _lock)
	{
		// 変化がなければ何もしない
		if (_lock == mouse_locked_) return;

		mouse_locked_ = _lock;

		RECT rect{};
		::GetClientRect(hwnd_, &rect);

		// カーソルを非表示
		if (mouse_locked_) {
			::ShowCursor(FALSE);
			// カーソルを中央に
			POINT center = {
				(rect.right - rect.left) * 0.5f,
				(rect.bottom - rect.top) * 0.5f
			};
			::ClientToScreen(hwnd_, &center);
			// カーソルを中央に
			::SetCursorPos(center.x, center.y);

			// ウィンドウ内に制限
			RECT clipRect{};
			::GetClientRect(hwnd_, &clipRect);
			::MapWindowPoints(hwnd_, nullptr, (POINT*)&clipRect, 2);
			::ClipCursor(&clipRect);
		}
		// カーソルを表示
		else {
			::ShowCursor(TRUE);
			::ClipCursor(NULL); // 制限解除
		}
	}

	/**
	 * @brief フォーカス設定
	 * @param _focused フォーカスが当たっているか
	 */
	void InputSystem::SetFocus(bool _focused)
	{
		if (_focused == focused_) return;
		focused_ = _focused;

		// フォーカスが外れた時
		if (!focused_) {
			// 押されているキーを全て離す
			::memset(keys_state_, 0, sizeof(keys_state_));
			::memset(old_keys_state_, 0, sizeof(old_keys_state_));
			// マウスデルタをリセット
			mouse_delta_ = {};
		}
		// フォーカスが当たった時
		else {
			// キーステートをリセット
			::memset(keys_state_, 0, sizeof(keys_state_));
			::memset(old_keys_state_, 0, sizeof(old_keys_state_));

			// マウスデルタをリセット
			mouse_delta_ = {};
			first_time_ = true;
		}
	}

	/**
	 * @brief 入力の有効/無効設定
	 * @param _enable 有効にするか
	 */
	void InputSystem::SetInputEnabled(bool _enable)
	{
		input_enabled_ = _enable;
		if (!input_enabled_) {
			// 押されているキーを全て離す
			::memset(keys_state_, 0, sizeof(keys_state_));
			::memset(old_keys_state_, 0, sizeof(old_keys_state_));
			// マウスデルタをリセット
			mouse_delta_ = {};
		}
	}

	/**
	 * @brief 入力が有効か
	 * @return 有効: true, 無効: false
	 */
	bool InputSystem::IsInputEnabled() const
	{
		return input_enabled_;
	}

	/**
	 * @brief RawInputの登録
	 */
	void InputSystem::RegisterRawMouse()
	{
		if (raw_mouse_registered_) { return; }

		RAWINPUTDEVICE rid{};
		rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid.usUsage = HID_USAGE_GENERIC_MOUSE;
		rid.dwFlags = RIDEV_NOLEGACY | /* RIDEV_INPUTSINK | */ RIDEV_CAPTUREMOUSE;	// フォーカス中でも取得したいならInputsinkをつける
		rid.hwndTarget = hwnd_;

		if (::RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
			raw_mouse_registered_ = true;
		}
		else {
			// 登録失敗
			use_raw_mouse_ = false;
		}
	}

	/**
	 * @brief フレーム開始処理
	 */
	void InputSystem::ClearFrameMouse()
	{
		// マウスの移動量を更新
		mouse_delta_ = raw_mouse_accum_;
		// リセット
		raw_mouse_accum_ = {};
		wheel_delta_ = 0.0f;
	}

}