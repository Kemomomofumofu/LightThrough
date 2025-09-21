#pragma once
/**
 * @file SceneManager.h
 * @brief Sceneの管理クラス
 * @author Arima Keita
 * @date 2025-09-15
 */

 // ---------- インクルード ---------- //
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include <optional>
#include <DX3D/Core/Base.h>

#include <Game/Scene/Scene.h>
#include <Game/ECS/Coordinator.h>
#include <Game/ECS/Entity.h>

namespace scene {
	struct SceneManagerDesc {
		dx3d::BaseDesc base;
		ecs::Coordinator& ecs;
	};

	class SceneManager: public dx3d::Base {
	public:
		using OnSceneEvent = std::function<void(const Scene::Id&)>;

		SceneManager(const SceneManagerDesc& _base);

		// Sceneの生成・削除
		Scene::Id CreateScene(const std::string& _name);
		bool UnloadScene(Scene::Id _id, bool _destroyEntities = true);

		// JSONで保存・読み込み
		bool SaveActiveScene(const std::string& _path);
		bool LoadSceneFromFile(const std::string& _path);

		// アクティベート
		bool SetActiveScene(Scene::Id _id, bool _unloadPrev = true);
		std::optional<Scene::Id> GetActiveScene() const;

		// Entityの管理
		void AddEntityToScene(const Scene::Id& _id, ecs::Entity _e);
		void RemoveEntityFromScene(const Scene::Id& _id, ecs::Entity _e);
		const std::vector<ecs::Entity>& GetEntitiesInScene(const Scene::Id& _id) const;

		// 永続化
		void MarkPersistentEntity(ecs::Entity _e, bool _persistent = true); // Entityを永続化するかどうか

		// イベント [ToDo] まだ仮置き
		//OnSceneEvent OnBeforeSceneUnload;
		//OnSceneEvent OnAfterSceneUnload;
		//OnSceneEvent OnBeforeSceneLoad;
		//OnSceneEvent OnAfterSceneLoad;

	private:
		Scene::Id GenerateId(const std::string& _base);

	private:
		ecs::Coordinator& ecs_;
		std::unordered_map<Scene::Id, Scene> scenes_{};
		std::optional<Scene::Id> active_scene_{};
		std::unordered_set<ecs::Entity> persistent_entities_{};
	};
}
