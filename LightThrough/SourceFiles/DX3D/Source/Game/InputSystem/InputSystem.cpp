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

namespace
{
	// マウスボタンが押されているか
	bool GetMouseButtonDown(input::MouseButton _btn)
	{
		int vk = 0;
		switch (_btn) {
		case input::MouseButton::Left: vk = VK_LBUTTON; break;
		case input::MouseButton::Right: vk = VK_RBUTTON; break;
		case input::MouseButton::Middle: vk = VK_MBUTTON; break;
		default: return false;
		}

		return (::GetAsyncKeyState(vk) & 0x8000) != 0;
	}
}

namespace input
{
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

		mouse_mode_ = MouseMode::Camera;
		ApplyModeState();
	}

	void InputSystem::Update()
	{
		if (!focused_ || !input_enabled_) {
			mouse_delta_ = {};

			::memset(mouse_buttons_, 0, sizeof(mouse_buttons_));
			::memset(old_mouse_buttons_, 0, sizeof(old_mouse_buttons_));
			return;
		}

		// ---------- キーボード ---------- //
		::memcpy(old_keys_state_, keys_state_, sizeof(keys_state_));
		::GetKeyboardState(keys_state_);

		// ---------- マウス ---------- //
		// マウスボタン
			UpdateMouseButtons();
		// マウス移動
		if (use_raw_mouse_) {
			ClearFrameMouse();
		}

		if (mouse_mode_ == MouseMode::Cursor) {
			
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

	bool InputSystem::IsMouseDown(MouseButton _button) const
	{
		const size_t i = static_cast<size_t>(_button);
		return mouse_buttons_[i];
	}

	bool InputSystem::IsMouseTrigger(MouseButton _button) const
	{
		const size_t i = static_cast<size_t>(_button);
		return (mouse_buttons_[i] && !old_mouse_buttons_[i]);
	}

	bool InputSystem::IsMouseRelease(MouseButton _button) const
	{
		const size_t i = static_cast<size_t>(_button);
		return (!mouse_buttons_[i] && old_mouse_buttons_[i]);
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
		if (!input_enabled_ || !use_raw_mouse_) { return; }

		UINT size = 0;
		if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(_lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) != 0 || size == 0) { return; }	// サイズ取得失敗

		raw_buffer_.resize(size);
		if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(_lParam), RID_INPUT, raw_buffer_.data(), &size, sizeof(RAWINPUTHEADER)) != size) { return; }	// データ取得失敗

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
	}

	/**
	 * @brief RawInputが有効か
	 * @return 有効: true, 無効: false
	 */
	bool InputSystem::IsRawMouseEnabled() const
	{
		return use_raw_mouse_;
	}


	void input::InputSystem::SetRelativeMouseMode(bool _enable)
	{
		if (relative_mouse_mode_ == _enable) { return; }
		relative_mouse_mode_ = _enable;

		if (_enable) {
			SetMouseMode(MouseMode::Camera);
		}
		else {
			if (mouse_mode_ == MouseMode::Camera) {
				SetMouseMode(MouseMode::Cursor);
			}
		}
	}

	bool InputSystem::IsRelativeMouseMode() const
	{
		return relative_mouse_mode_;
	}

	void InputSystem::SetMouseMode(MouseMode _mode)
	{
		if (mouse_mode_ == _mode) { return; }
		mouse_mode_ = _mode;
		ApplyModeState();
	}

	MouseMode InputSystem::GetMouseMode() const
	{
		return mouse_mode_;
	}

	/**
	 * @brief マウス固定メソッド
	 * @param _lock 固定するかしないか
	 *
	 * 	[ToDo] マウスカーソルの固定する、しない、をイベント形式にしたいが、
	 *	WinProcのSetFocusで消す、KillFocusで戻す、だと、
	 *	ゲームがメニュー画面でもマウスが消えてしまうので、今後考える。
	 */
	void InputSystem::MouseLock(bool _lock)
	{
		if (_lock == mouse_locked_) return;
		mouse_locked_ = _lock;

		if (mouse_locked_)
		{
			::ShowCursor(FALSE);
			UpdateCursorClip();
		}
		// カーソルを表示
		else {
			mouse_delta_ = {};
			::ShowCursor(TRUE);
			::ClipCursor(NULL);
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
		::memset(keys_state_, 0, sizeof(keys_state_));
		::memset(old_keys_state_, 0, sizeof(old_keys_state_));
		mouse_delta_ = {};


		if (!focused_) {
			if (mouse_locked_) {
				MouseLock(false);
			}
			else {
				// カメラモードはロック維持
				if (mouse_mode_ == MouseMode::Camera && !mouse_locked_) {
					MouseLock(true);
				}
			}
		}
		else {
			MouseLock(false);
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

	void InputSystem::EnterGameplay(bool _relative)
	{
		SetInputEnabled(true);
		SetFocus(true);
		if (_relative) {
			SetRelativeMouseMode(true);
		}
		else {
			SetRelativeMouseMode(false);
		}
	}

	void InputSystem::ReleaseToDesktop()
	{
		SetRelativeMouseMode(false);
		SetInputEnabled(false);
		SetFocus(false);
		MouseLock(false);
		::ClipCursor(nullptr);
		::ShowCursor(TRUE);
	}

	void InputSystem::UpdateCursorClip()
	{
		if (!hwnd_) { return; }
		if (!focused_ || !input_enabled_) {
			::ClipCursor(nullptr);
			return;
		}

		// クライアント領域
		RECT rect{};
		::GetClientRect(hwnd_, &rect);

		switch (mouse_mode_)
		{
		case MouseMode::Camera:
		{
			// 画面中央にクリップ
			POINT center = {
				(rect.right - rect.left) * 0.5f,
				(rect.bottom - rect.top) * 0.5f
			};
			::ClientToScreen(hwnd_, &center);

			// 1px固定
			RECT clip{ center.x, center.y, center.x + 1, center.y + 1 };
			::ClipCursor(&clip);
			break;
		}
		case MouseMode::Cursor:
		{
			// クライアント領域にクリップ
			POINT tl{ rect.left, rect.top };
			POINT br{ rect.right, rect.bottom };
			::ClientToScreen(hwnd_, &tl);
			::ClientToScreen(hwnd_, &br);
			RECT clip{ tl.x, tl.y, br.x, br.y };
			::ClipCursor(&clip);
			break;
		}
		}
	}

	/**
	 * @brief RawInputの登録
	 */
	void InputSystem::ReRegisterRawMouse(bool _gameplay)
	{
		// 既存の登録をいったん削除(必要かどうかはわからないが切り替えできるようになるのかな)
		if (raw_mouse_registered_) {
			RAWINPUTDEVICE remove{};
			remove.usUsagePage = HID_USAGE_PAGE_GENERIC;
			remove.usUsage = HID_USAGE_GENERIC_MOUSE;
			remove.dwFlags = RIDEV_REMOVE;
			remove.hwndTarget = nullptr;
			::RegisterRawInputDevices(&remove, 1, sizeof(remove));
			raw_mouse_registered_ = false;
		}

		if (!use_raw_mouse_) { return; }

		RAWINPUTDEVICE rid{};
		rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid.usUsage = HID_USAGE_GENERIC_MOUSE;
		rid.hwndTarget = hwnd_;
		rid.dwFlags = _gameplay ? (RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE) : 0;

		if (::RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
			raw_mouse_registered_ = true;
		}
		else {
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

	//! @brief マウスボタン状態更新
	void InputSystem::UpdateMouseButtons()
	{
		// 保存
		::memcpy(old_mouse_buttons_, mouse_buttons_, sizeof(mouse_buttons_));
		// 更新
		for(size_t i = 0; i< static_cast<size_t>(MouseButton::Max); ++i) {
			mouse_buttons_[i] = GetMouseButtonDown(static_cast<MouseButton>(i));
		}
	}

	/**
	 * @brief マウスモードに応じた状態を適用
	 *
	 * - c/ Camera: 入力有効、フォーカス有効、RawInput有効、マウスロック
	 * - c/ Cursor: 入力無効、フォーカス有効、RawInput無効、マウスロック解除
	 * - c/ Disabled: すべて無効
	 */
	void InputSystem::ApplyModeState()
	{
		switch (mouse_mode_) {
		case MouseMode::Camera:
		{
			input_enabled_ = true;
			focused_ = true;
			use_raw_mouse_ = true;
			ReRegisterRawMouse(true);
			MouseLock(true);
			break;
		}
		case MouseMode::Cursor:
		{
			input_enabled_ = true;
			focused_ = true;
			use_raw_mouse_ = false;
			ReRegisterRawMouse(false);
			MouseLock(false);
			break;
		}
		case MouseMode::Disabled:
		{
			input_enabled_ = false;
			focused_ = false;
			use_raw_mouse_ = false;
			relative_mouse_mode_ = false;
			ReRegisterRawMouse(false);
			MouseLock(false);
			break;
		}
		}
		// 入力無効化時はバッファクリア
		if (!input_enabled_) {
			::memset(keys_state_, 0, sizeof(keys_state_));
			::memset(old_keys_state_, 0, sizeof(old_keys_state_));
			mouse_delta_ = {};
			raw_mouse_accum_ = {};
		}
	}

}