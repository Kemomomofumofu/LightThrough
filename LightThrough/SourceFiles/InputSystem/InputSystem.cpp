/**
 * @file InputSystem.cpp
 * @brief 入力を管理するクラス
 * @author Arima Keita
 * @date 2025-07-30
 */

 // ---------- インクルード ---------- // 
#include <InputSystem/InputSystem.h>
#include <Windows.h>

/**
 * @brief リスナー追加
 * @param _listener 追加するリスナー
 */
void LightThrough::InputSystem::AddListener(InputListener* _listener)
{
	listeners_.insert(_listener);
}

/**
 * @brief リスナー削除
 * @param _listener 削除するリスナー
 */
void LightThrough::InputSystem::RemoveListener(InputListener* _listener)
{
	listeners_.erase(_listener);
}

/**
 * @brief 更新処理
 */
void LightThrough::InputSystem::Update()
{
	POINT currentMousePos = {};
	::GetCursorPos(&currentMousePos);

	if (first_time_) {
		old_mouse_pos_ = dx3d::Point(currentMousePos.x, currentMousePos.y);
		first_time_ = false;
	}

	if (currentMousePos.x != old_mouse_pos_.x || currentMousePos.y != old_mouse_pos_.y) {
		// マウスが動いた際の処理
		std::unordered_set<InputListener*>::iterator itr = listeners_.begin();

		while (itr != listeners_.end()) {
			(*itr)->OnMouseMove(dx3d::Point(currentMousePos.x - old_mouse_pos_.x, currentMousePos.y - old_mouse_pos_.y));
			++itr;
		}
	}
	old_mouse_pos_ = dx3d::Point(currentMousePos.x, currentMousePos.y);


	if (::GetKeyboardState(keys_state_)) {
		for (unsigned int i = 0; i < 256; ++i) {
			// もし押されていたら
			if (keys_state_[i] & 0x80) {
				std::unordered_set<InputListener*>::iterator itr = listeners_.begin();

				while (itr != listeners_.end()) {
					(*itr)->OnKeyDown(i);
					++itr;
				}
			}
			else {
				if (keys_state_[i] != old_keys_state_[i]) {
					std::unordered_set<InputListener*>::iterator itr = listeners_.begin();

					while (itr != listeners_.end()) {
						(*itr)->OnKeyUp(i);
						++itr;
					}
				}
			}
		}
		// 前回のキーステートを保存
		::memcpy(old_keys_state_, keys_state_, sizeof(unsigned char) * 256);
	}
}


/**
 * @brief ゲッター
 * @return インプットシステムの参照
 */
LightThrough::InputSystem& LightThrough::InputSystem::Get() {
	static InputSystem system;
	return system;
}