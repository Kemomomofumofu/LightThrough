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

#include <Game/Scene/SceneData.h>
#include <Game/Scene/SceneSerializer.h>
#include <Game/ECS/Coordinator.h>
#include <Game/ECS/Entity.h>

namespace scene {
	struct SceneManagerDesc {
		dx3d::BaseDesc base;
		ecs::Coordinator& ecs;
	};

	/**
	 * @brief Scene管理クラス。
	 *
	 * シーンはイベントフックで切り替える。
	 */
	class SceneManager: public dx3d::Base {
	public:
		using OnSceneEvent = std::function<void(const SceneData::Id&)>;

		OnSceneEvent OnAfterSceneLoad;
		OnSceneEvent OnBeforeSceneUnload;


		SceneManager(const SceneManagerDesc& _base);

		// Sceneの生成・削除
		SceneData::Id CreateScene(const std::string& _name);
		bool UnloadScene(SceneData::Id _id, bool _destroyEntities = true);

		// JSONで保存・読み込み
		bool SaveActiveScene();
		bool LoadSceneFromFile(const std::string& _name);

		/**
		 * @brief Scene切り替え
		 * @param _newScene 新しいシーンID
		 * @return 成功: True、失敗: False
		 */
		bool ChangeScene(const SceneData::Id& _newScene, bool _unloadPrev = true);

		/**
		 * @brief アクティブなシーンをリロード
		 * @return 成功: True、失敗: False
		 */
		bool ReloadActiveScene();

		// アクティベート
		bool SetActiveScene(const SceneData::Id& _id, bool _unloadPrev = true);
		std::optional<SceneData::Id> GetActiveScene() const;

		// Entityの管理
		void AddEntityToScene(const SceneData::Id& _id, ecs::Entity _e);
		void RemoveEntityFromScene(const SceneData::Id& _id, ecs::Entity _e);
		const std::vector<ecs::Entity>& GetEntitiesInScene(const SceneData::Id& _id) const;

		// 永続化
		void MarkPersistentEntity(ecs::Entity _e, bool _persistent = true); // Entityを永続化するかどうか


		// イベント [ToDo] まだ仮置き
		//OnSceneEvent OnAfterSceneUnload;
		//OnSceneEvent OnBeforeSceneLoad;

	private:
		SceneData::Id GenerateId(const std::string& _base);

	private:
		ecs::Coordinator& ecs_;
		std::unordered_map<SceneData::Id, SceneData> scenes_{};
		std::optional<SceneData::Id> active_scene_{};
		std::unordered_set<ecs::Entity> persistent_entities_{};
		std::unique_ptr<ecs_serial::SceneSerializer> serializer_{};



	// ---------- デバッグ関連 ---------- // 
	private:
		void DebugCurrentScene();
	private:
		std::optional<ecs::Entity> debug_selected_entity_{};
	};
}
