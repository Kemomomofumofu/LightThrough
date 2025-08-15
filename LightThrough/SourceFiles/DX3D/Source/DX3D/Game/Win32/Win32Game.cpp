/**
 * @file Win32Game.cpp
 * @brief ゲームループがある
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Game/Game.h>
#include <Windows.h>


/**
 * @brief ゲームの実行
 */
void dx3d::Game::Run() {
	MSG msg{};
	while (is_running_) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				is_running_ = false;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		OnInternalUpdate();
	}
}