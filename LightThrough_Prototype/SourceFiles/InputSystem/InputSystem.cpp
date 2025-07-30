/**
 * @file InputSystem.cpp
 * @brief 入力を管理するクラス
 * @author Arima Keita
 * @date 2025-07-30
 */

// ---------- インクルード ---------- // 
#include <InputSystem/InputSystem.h>

LightThrough::InputSystem::InputSystem()
{
}

LightThrough::InputSystem::~InputSystem()
{
}

void LightThrough::InputSystem::AddListener(InputListener* _listener)
{
}

void LightThrough::InputSystem::RemoveListener(InputListener* _listener)
{
}

void LightThrough::InputSystem::NotifyListener(InputListener* _listener)
{
}


LightThrough::IInputSystem* LightThrough::InputSystem::Get() {
	// if()
}