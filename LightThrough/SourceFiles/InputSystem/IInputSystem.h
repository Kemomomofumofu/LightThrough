#pragma once
/**
 * @file IInputSystem.h
 * @brief 入力システムのインターフェース
 * @author Arima Keita
 * @date 2025-07-30
 */

// ---------- インクルード ---------- // 


namespace input {
	struct IInputSystem {
		virtual ~IInputSystem() = default;
		virtual void Update() = 0;
		virtual bool IsKeyDown(int _key) const = 0;
	};
}