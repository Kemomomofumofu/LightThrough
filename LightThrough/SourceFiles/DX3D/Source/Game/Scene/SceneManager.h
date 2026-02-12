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
	class SceneManager : public dx3d::Base {
	public:
		using OnSceneEvent = std::function<void(const SceneData::Id&)>;

		OnSceneEvent OnAfterSceneLoad;
		OnSceneEvent OnBeforeSceneUnload;


		SceneManager(const SceneManagerDesc& _base);

		/**
		 * @brief SceneDataの生成
		 * @param _name		SceneData名
		 * @return 生成したSceneDataのID
		 */
		SceneData::Id CreateScene(const std::string& _name);

		/**
		 * @brief SceneDataをアンロードする
		 * @param _id	シーンID
		 * @param _destroyEntities	シーンに含まれるEntityを破棄するかどうか
		 * @return 成功: true、失敗: false
		 */
		bool UnloadScene(SceneData::Id _id, bool _destroyEntities = true);

		/**
		 * @brief アクティブなSceneDataを保存する
		 * @return 成功: true, 失敗: false
		 */
		bool SaveActiveScene();
		/**
		* @brief ファイルからSceneDataを読み込む
		* @param _path	: ファイルパス
		* @param _id	: シーンID
		* @return 成功: True, 失敗: False
		*/
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

		/**
		 * @brief SceneDataをアクティブにする
		 * @param _id		: シーンID
		 * @param unloadPrev: 前のシーンをアンロードするかどうか
		 * @return 成功: True、失敗: False
		 */
		bool SetActiveScene(const SceneData::Id& _id, bool _unloadPrev = true);

		/**
		 * @brief アクティブなSceneDataのIDを取得
		 * @return アクティブなSceneDataのID, 無い場合: nullopt
		 */
		std::optional<SceneData::Id> GetActiveScene() const;

		/**
		 * @brief SceneDataにEntityを追加
		 * @param _id	シーンID
		 * @param _e	追加するEntity
		 */
		void AddEntityToScene(const SceneData::Id& _id, ecs::Entity _e);
		/**
		 * @brief SceneDataからEntityを削除
		 * @param _id シーンID
		 * @param _e 削除するEntity
		 */
		void RemoveEntityFromScene(const SceneData::Id& _id, ecs::Entity _e);
		/**
		 * @brief SceneDataに含まれるEntity一覧を取得
		 * @param _id: シーンID
		 * @return 含まれるEntity一覧
		 */
		const std::vector<ecs::Entity>& GetEntitiesInScene(const SceneData::Id& _id) const;

		/**
		 * @brief Entityを永続化するかどうかを設定
		 * @param _e			: Entity
		 * @param _persistent	: 永続化するかどうか
		 */
		void MarkPersistentEntity(ecs::Entity _e, bool _persistent = true); // Entityを永続化するかどうか

		/**
		 * @brief Entity破棄時コールバック
		 * @param _e 破棄されたEntity
		 */
		void OnEntityDestroyed(ecs::Entity _e);

		// イベント [ToDo] まだ仮置き
		//OnSceneEvent OnAfterSceneUnload;
		//OnSceneEvent OnBeforeSceneLoad;

	private:
		/**
		 * @brief シーンIDを生成
		 * @param _base	ベースとなる名前
		 * @return ユニークなシーンID
		 */
		SceneData::Id GenerateId(const std::string& _base);

	private:
		ecs::Coordinator& ecs_;
		std::unordered_map<SceneData::Id, SceneData> scenes_{};		// シーン一覧
		std::optional<SceneData::Id> active_scene_{};				// アクティブなシーンID
		std::unordered_set<ecs::Entity> persistent_entities_{};		// 永続化するEntity一覧
		std::unique_ptr<ecs_serial::SceneSerializer> serializer_{};	// シーンシリアライザー



		// ---------- デバッグ関連 ---------- // 
	private:
		void DebugCurrentScene();
	private:
		std::optional<ecs::Entity> debug_selected_entity_{};
	};
}
