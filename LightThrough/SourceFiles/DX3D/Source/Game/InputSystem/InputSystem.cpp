/**
 * @file InputSystem.cpp
 * @brief 入力を管理するクラス
 * @author Arima Keita
 * @date 2025-07-30
 */

 // ---------- インクルード ---------- // 
#include <Game/InputSystem/InputSystem.h>
#include <Windows.h>


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

		// マウスの初期状態
		LockMouse(true);
		input_enabled_ = true;
	}

	void InputSystem::Update()
	{
		if (!focused_) return;

		// ---------- キーボード ---------- //
		::memcpy(old_keys_state_, keys_state_, sizeof(keys_state_));
		if (::GetKeyboardState(keys_state_)) {
			for (unsigned int i = 0; i < 256; ++i) {
				bool nowDown = (keys_state_[i] & 0x80) != 0;
				bool wasDown = (old_keys_state_[i] & 0x80) != 0;
			}
		}

		// ---------- マウス ---------- //
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
				(rect.right  - rect.left) * 0.5f,
				(rect.bottom - rect.top)  * 0.5f
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
}


///**
// * @brief 更新処理
// */
//void InputSystem::Update()
//{
//	// フォーカスが当たっていなければ入力を取らない
//	if (!focused_) return;
//
//	// ---------- キーボード ---------- // 
//	::memcpy(old_keys_state_, keys_state_, sizeof(keys_state_));
//	if (::GetKeyboardState(keys_state_)) {
//		for (unsigned int i = 0; i < 256; ++i) {
//			bool nowDown = (keys_state_[i] & 0x80) != 0;		// 今押されているか
//			bool wasDown = (old_keys_state_[i] & 0x80) != 0;	// 前回押されていたか
//
//			// 押された瞬間だったら
//			if (nowDown && !wasDown) {
//				for (auto* l : listeners_) {
//					l->OnKeyDown(i);	// 押されたキーを通知
//				}
//			}
//			// 離された瞬間だったら
//			else if (!nowDown && wasDown) {
//				for (auto* l : listeners_) {
//					l->OnKeyUp(i);		// 離されたキーを通知
//				}
//			}
//		}
//	}
//
//	// ---------- マウス ---------- //
//	// マウスが固定されている場合
//	if (mouse_locked_) {
//		// カーソルを中央固定にして相対移動だけ渡す
//		POINT current{};
//		::GetCursorPos(&current);
//
//		// ウィンドウの中心を計算
//		RECT clientRect{};
//		::GetClientRect(hwnd_, &clientRect);
//		POINT center{
//			(clientRect.right - clientRect.left) * 0.5f,
//			(clientRect.bottom - clientRect.top) * 0.5f
//		};
//		// クライアント座標からスクリーン座標に変換
//		::ClientToScreen(hwnd_, &center);
//
//		// 移動量
//		dx3d::Point delta{ current.x - center.x, current.y - center.y };
//		if (delta.x || delta.y) {
//			for (auto* l : listeners_) {
//				l->OnMouseMove(delta);
//			}
//		}
//		// カーソルを中央に戻す
//		::SetCursorPos(center.x, center.y);
//	}
//	// マウスが固定されていない場合
//	else {
//		POINT current{};
//		::GetCursorPos(&current);
//		// スクリーン座標からクライアント座標に変換
//		::ScreenToClient(hwnd_, &current);
//
//		for (auto* l : listeners_) {
//			l->OnMouseMove({ current.x, current.y });
//		}
//	}
//}
//
///**
// * @brief リスナー追加
// * @param _listener 追加するリスナー
// */
//void InputSystem::AddListener(InputListener* _listener)
//{
//	listeners_.insert(_listener);
//}
//
///**
// * @brief リスナー削除
// * @param _listener 削除するリスナー
// */
//void InputSystem::RemoveListener(InputListener* _listener)
//{
//	listeners_.erase(_listener);
//}