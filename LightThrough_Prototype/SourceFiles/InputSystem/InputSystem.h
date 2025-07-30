#pragma once
/**
 * @file InputSystem
 * @brief 入力を管理するクラス(シングルトン)
 * @author Arima Keita
 * @date 2025-07-30
 */

// ---------- インクルード ---------- // 
#include <InputSystem/IInputSystem.h>
#include <InputSystem/InputListener.h>

namespace LightThrough {

	/**
	 * @brief 入力システム
	 *
	 * 入力を管理するクラス。
	 * [ToDo] 現状は工数削減のためにシングルトンでの実装。
	 * 将来的にはDIなどで明示的にしたい。
	 */
	class InputSystem : public IInputSystem
	{
	public:
		InputSystem();
		~InputSystem();

		void AddListener(InputListener* _listener);
		void RemoveListener(InputListener* _listener);
		void NotifyListener(InputListener* _lilstener);

		static IInputSystem* Get();
	};

}

