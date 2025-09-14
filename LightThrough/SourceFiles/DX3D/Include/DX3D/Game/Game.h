#pragma once
/**
 * @file Game.h
 * @brief ゲームの実行をする
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <memory>
#include <chrono>
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Core.h>
#include <InputSystem/InputListener.h>
#include <DX3D/Game/ECS/Coordinator.h>



namespace dx3d {
	// ---------- 前方宣言 ---------- //
	/**
	 * @brief ゲームクラス
	 *
	 * ゲームの実行、更新をするクラス
	 * [ToDo] InputListenerについては各オブジェクトに持たせたほうが責務がわかりやすい。
	 */
	class Game :public Base{
	public:
		explicit Game(const GameDesc& _desc);
		virtual ~Game() override;

		virtual void Run() final;

	private:
		void OnInternalUpdate();
	private:
		std::unique_ptr<Logger>logger_ptr_{};
		std::unique_ptr<GraphicsEngine> graphics_engine_{};
		std::unique_ptr<Display> display_{};
		bool is_running_ = true;

		std::unique_ptr<ecs::Coordinator> ecs_coordinator_{};	// ECSのコーディネーター
		std::chrono::high_resolution_clock::time_point last_time_{};	// 時間管理用

	};
}