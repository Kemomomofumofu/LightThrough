/**
 * @file InputSystem.cpp
 * @brief 入力を管理するクラス
 * @author Arima Keita
 * @date 2025-07-30
 */

 // ---------- インクルード ---------- // 
#include <InputSystem/InputSystem.h>
#include <Windows.h>


namespace input {
	/**
	 * @brief リスナー追加
	 * @param _listener 追加するリスナー
	 */
	void InputSystem::AddListener(InputListener* _listener)
	{
		listeners_.insert(_listener);
	}

	/**
	 * @brief リスナー削除
	 * @param _listener 削除するリスナー
	 */
	void InputSystem::RemoveListener(InputListener* _listener)
	{
		listeners_.erase(_listener);
	}

	void InputSystem::Init(HWND _hwnd)
	{
		hwnd_ = _hwnd;
	}

	/**
	 * @brief 更新処理
	 */
	void InputSystem::Update()
	{
		// フォーカスが当たっていなければ入力を取らない
		if (!focused_) return;

		// ---------- キーボード ---------- // 
		::memcpy(old_keys_state_, keys_state_, sizeof(keys_state_));
		if (::GetKeyboardState(keys_state_)) {
			for (unsigned int i = 0; i < 256; ++i) {
				bool nowDown = (keys_state_[i] & 0x80) != 0;		// 今押されているか
				bool wasDown = (old_keys_state_[i] & 0x80) != 0;	// 前回押されていたか

				// 押された瞬間だったら
				if (nowDown && !wasDown) {
					for (auto* l : listeners_) {
						l->OnKeyDown(i);	// 押されたキーを通知
					}
				}
				// 離された瞬間だったら
				else if (!nowDown && wasDown) {
					for (auto* l : listeners_) {
						l->OnKeyUp(i);		// 離されたキーを通知
					}
				}
			}
		}

		// ---------- マウス ---------- //
		// マウスが固定されている場合
		if (mouse_locked_) {
			// カーソルを中央固定にして相対移動だけ渡す
			POINT current{};
			::GetCursorPos(&current);

			// ウィンドウの中心を計算
			RECT clientRect{};
			::GetClientRect(hwnd_, &clientRect);
			POINT center{
				(clientRect.right  - clientRect.left) * 0.5f,
				(clientRect.bottom - clientRect.top)  * 0.5f
			};
			// クライアント座標からスクリーン座標に変換
			::ClientToScreen(hwnd_, &center);

			// 移動量
			dx3d::Point delta{ current.x - center.x, current.y - center.y };
			if (delta.x || delta.y) {
				for (auto* l : listeners_) {
					l->OnMouseMove(delta);
				}
			}
			// カーソルを中央に戻す
			::SetCursorPos(center.x, center.y);
		}
		// マウスが固定されていない場合
		else {
			POINT current{};
			::GetCursorPos(&current);
			// スクリーン座標からクライアント座標に変換
			::ScreenToClient(hwnd_, &current);

			for(auto* l : listeners_) {
				l->OnMouseMove({ current.x, current.y});
			}
		}
	}

	/**
	 * @brief ゲッター
	 * @return インプットシステムの参照
	 */
	InputSystem& InputSystem::Get() {
		static InputSystem system;
		return system;
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
			for(unsigned int i = 0; i < 256; ++i) {
				if (keys_state_[i] & 0x80) {
					for (auto* l : listeners_) {
						l->OnKeyUp(i);		// 離されたキーを通知
					}
				}
			}
			::memset(keys_state_, 0, sizeof(keys_state_));
			::memset(old_keys_state_, 0, sizeof(old_keys_state_));
		}
		// フォーカスが当たった時
		else {
			// キーステートをリセット
			::memset(keys_state_, 0, sizeof(keys_state_));
			::memset(old_keys_state_, 0, sizeof(old_keys_state_));
		}
	}
}