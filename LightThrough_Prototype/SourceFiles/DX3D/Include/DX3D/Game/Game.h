#pragma once
/**
 * @file Game.h
 * @brief ゲームの実行をする
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Core.h>



/**
 * @brief ゲームクラス
 *
 * ゲームの実行、更新をするクラス
 */
namespace dx3d {
	class Game :public Base {
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

	};
}