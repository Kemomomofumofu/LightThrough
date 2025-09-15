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

#include <DX3D/Game/Scene/Scene.h>
#include <DX3D/Game/ECS/Coordinator.h>
#include <DX3D/Game/ECS/Entity.h>

namespace scene {
	class SceneManager: public dx3d::Base {
	public:
		using OnSceneEvent = std::function<void(const Scene::Id&)>;

		SceneManager(dx3d::BaseDesc _base) : dx3d::Base(_base) {}

		void Init(ecs::Coordinator& _ecs);	// 初期化

		// Sceneの生成・削除
		Scene::Id CreateScene(const std::string& _name);
		bool UnloadScene(Scene::Id _id, bool _destroyEntities = true);

		// JSONで保存・読み込み
		bool LoadSceneFromFile(const std::string& _path, Scene::Id _id);
		bool SaveSceneToFile(const Scene::Id& _id, const std::string& _path);

		// アクティベート
		bool SetActiveScene(Scene::Id _id, bool _unloadPrev = true);
		std::optional<Scene::Id> GetActiveScene() const;

		// Entityの管理
		void AddEntityToScene(const Scene::Id& _id, ecs::Entity _e);
		void RemoveEntityFromScene(const Scene::Id& _id, ecs::Entity _e);
		const std::vector<ecs::Entity>& GetEntitiesInScene(const Scene::Id& _id) const;

		// 永続化
		void MarkPersistentEntity(ecs::Entity _e, bool _persistent = true); // Entityを永続化するかどうか

		// イベント
		OnSceneEvent OnBeforeSceneUnload;
		OnSceneEvent OnAfterSceneUnLoad;
		OnSceneEvent OnBeforeSceneLoad;
		OnSceneEvent OnAfterSceneLoad;

		// コンポーネントのデシリアライズ関数の登録
		using ComponentDeserializer = std::function<void(ecs::Coordinator&, ecs::Entity, const std::string&)>;
		void RegisterComponentDeserializer(const std::string& _componentName, ComponentDeserializer _deserializer);

	private:
		Scene::Id GenerateId(const std::string& _base);

	private:
		ecs::Coordinator* ecs_{};
		std::unordered_map<Scene::Id, Scene> scenes_{};
		std::optional<Scene::Id> active_scene_{};
		std::unordered_set<ecs::Entity> persistent_entities_{};

		// コンポーネントのデシリアライズ関数マップ
		std::unordered_map<std::string, ComponentDeserializer> component_deserializers_{};
	};
}
