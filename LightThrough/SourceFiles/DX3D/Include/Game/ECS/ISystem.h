#pragma once
/**
 * @file ISystem.h
 * @brief Updateを持つシステムのインターフェース
 * @author Arima Keita
 * @date 2025-08-08
 */

// ---------- インクルード ---------- // 
#include <set>
#include <DX3D/Core/Base.h>
#include <Game/GameLogUtils.h>

namespace ecs {
	// ---------- 前方宣言 ---------- //
	struct Entity;
	class Coordinator;


	class ISystem  : public dx3d::Base{
	public:
		explicit ISystem(const SystemDesc& _desc)
			: dx3d::Base(_desc.base)
			, engine_(_desc.graphicsEngine)
			, ecs_(_desc.ecs)
			, scene_manager_(_desc.sceneManager)
			, one_shot_(_desc.oneShot){}

		virtual ~ISystem() = default;
		virtual void Init() {}
		virtual void Update(float _dt) {}
		virtual void FixedUpdate(float _fixedDt) {}
		virtual void OnEntityDestroyed(Entity _e) {}
		virtual void OnSceneLoaded() {}

		/**
		 * @brief 一度だけ実行するシステムか
		 * @return true: 一度だけ実行, false: 毎フレーム実行
		 */
		bool IsOneShot() const { return one_shot_; }

		
		/**
		 * @brief activeの切り替え
		 * @param _active 有効にするか無効にするか
		 */
		void SetActive(bool _active) { active_ = _active; }

		std::set<Entity> entities_;	// 処理対象のEntityの集合

	protected:
		ecs::Coordinator& ecs_; // ecs::Coordinatorへの参照
		scene::SceneManager& scene_manager_;
		dx3d::GraphicsEngine& engine_;

		bool one_shot_ = false; // 一度だけ実行するシステムか
		bool active_ = true; // システムがアクティブか
	};
}