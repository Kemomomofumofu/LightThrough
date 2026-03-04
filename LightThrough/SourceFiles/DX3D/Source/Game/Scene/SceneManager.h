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

	struct SceneTransitionInfo {
		DirectX::XMFLOAT3 goalPosition{};	// 前シーンのゴール位置
		std::string startPointName = "StartLight";	// 次シーンのスタートEntity名
	};

	/**
	 * @brief Scene管理クラス。
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
		 * @brief Sceneの保存
		 * @param _id 保存するSceneDataのID
		 * @return 成功: true, 失敗: false
		 */
		bool SaveScene(const SceneData::Id& _id);

		/**
		* @brief ファイルからSceneDataを読み込む
		* @param _path	: ファイルパス
		* @param _id	: シーンID
		* @return 成功: True, 失敗: False
		*/
		bool LoadSceneFromFile(const std::string& _name);

		/**
		 * @brief シーンをプリロードする
		 * @param _name : シーンID
		 * @return 成功: True, 失敗: False
		 */
		bool PreloadScene(const std::string& _name);

		/**
		 * @brief プリロードされたシーンを有効化する
		 * @param _name : シーン名
		 * @return 成功: True, 失敗: False
		 */
		bool ActivatePreloadedScene(const std::string& _name);

		/**
		 * @brief Scene切り替え
		 * @param _newScene : 新しいシーンID
		 * @return 成功: True、失敗: False
		 */
		bool ChangeScene(const SceneData::Id& _newScene, bool _unloadPrev = true);

		/**
		 * @brief Scene切り替えリクエスト
		 * @param _newScene : 新しいシーンID
		 */
		void RequestChangeScene(const SceneData::Id& _newScene);
		/**
		 * @brief Scene切り替えリクエスト（遷移情報付き）
		 * @param _newScene : 新しいシーンID
		 * @param _info : シーン遷移情報
		 */
		void RequestChangeScene(const SceneData::Id& _newScene, const SceneTransitionInfo& _info);

		/**
		 * @brief 保留中のシーン切り替えリクエストを実行
		 * @return 成功: True、失敗: False
		 */
		bool FlushSceneChangeRequest();

		/**
		 * @brief 保留中のシーン切り替えリクエストがあるか
		 * @return True: ある, False: ない
		 */
		bool HasPendingSceneChange() const { return pending_scene_change_.has_value(); }

		/**
		 * @brief シーンの追加
		 * @param _id 追加するシーンID
		 * @return 成功: True, 失敗: False
		 */
		bool AddScene(const SceneData::Id& _id);

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
		 * @brief Entity破棄時コールバック
		 * @param _e 破棄されたEntity
		 */
		void OnEntityDestroyed(ecs::Entity _e);

		/**
		 * @brief 保留中のシーン遷移情報を取得
		 * @return 保留中のシーン遷移情報, 無い場合: nullopt
		 */
		const std::optional<SceneTransitionInfo>& GetPendingTransitionInfo() const { return pending_transition_info_; }
		
		/**
		 * @brief 保留中のシーン遷移情報を削除
		 */
		void ClearPendingTransitionInfo() { pending_transition_info_.reset(); }

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
		std::unordered_map<std::string, SceneData> preloaded_scenes_{};	// プリロードされたシーン一覧
		std::optional<SceneData::Id> active_scene_{};				// アクティブなシーンID
		std::optional<SceneData::Id> pending_scene_change_{};		// 保留中のシーン変更
		std::optional<SceneTransitionInfo> pending_transition_info_{};	// 保留中のシーン遷移情報（シーン切り替えとセットで使用）
		std::unique_ptr<ecs_serial::SceneSerializer> serializer_{};	// シーンシリアライザー



		// ---------- デバッグ関連 ---------- // 
	private:
		void DebugCurrentScene();
	private:
		std::optional<ecs::Entity> debug_selected_entity_{};
		std::string debug_scene_name_input_{};
		std::string debug_load_name_input_{};
		std::optional<SceneData::Id> debug_selected_scene_{};
	};
}
