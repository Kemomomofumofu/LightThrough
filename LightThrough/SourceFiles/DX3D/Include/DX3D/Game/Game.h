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
#include <Game/ECS/Coordinator.h>
#include <Game/Scene/SceneManager.h>



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
		/**
		 * @brief 更新
		 */
		void OnInternalUpdate();

		/**
		 * @brief シーンの保存
		 */
		void SaveScene();
		/**
		 * @brief シーンのリロード
		 */
		void ReloadScene();
		/**
		 * @brief シーンの切り替え
		 * @param _newScene 新しいシーンID
		 */
		void ChangeScene(const scene::SceneData::Id& _newScene);
	private:
		std::unique_ptr<Logger>logger_ptr_{};
		std::unique_ptr<GraphicsEngine> graphics_engine_{};
		std::unique_ptr<Display> display_{};
		bool is_running_ = true;

		std::unique_ptr<ecs::Coordinator> ecs_coordinator_{};	// ECSのコーディネーター
		std::unique_ptr<scene::SceneManager> scene_manager_{};
		std::chrono::high_resolution_clock::time_point last_time_{};	// 時間管理用

		float accumulated_time_ = 0.0f; // 固定更新用の累積時間
		float fixed_time_step_ = 1.0f / 60.0f; // 固定更新の時間間隔

	};
}