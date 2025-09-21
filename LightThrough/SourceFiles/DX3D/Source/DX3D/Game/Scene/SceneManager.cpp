/**
 * @file SceneManager.cpp
 * @brief Scene管理クラス
 * @author Arima Keita
 * @date 2025-09-15
 */

 // ---------- インクルード ---------- // 
#include <fstream>
#include <sstream>
#include <cassert>
#include <nlohmann/json.hpp>
#include <DX3D/Game/ECS/Coordinator.h>
#include <DX3D/Game/Scene/SceneManager.h>
#include <DX3D/Game/Scene/SceneSerializer.h>

#include <Game/Components/Transform.h>
#include <Game/Components/Mesh.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>
//#include <Game/Components/Sprite.h>

#include <DX3D/Game/ECS/ECSLogUtils.h>

using json = nlohmann::json;

namespace scene {

	void SceneManager::Init(ecs::Coordinator& _ecs)
	{
		ecs_ = &_ecs;
	}



	/**
	 * @brief Sceneの生成
	 * @param _name		Scene名
	 * @return 生成したSceneのID
	 */
	Scene::Id SceneManager::CreateScene(const std::string& _name)
	{
		Scene::Id id = GenerateId(_name);
		Scene s;
		s.id_ = id;
		s.name_ = _name;
		scenes_.emplace(id, std::move(s));
		return id;
	}

	/**
	 * @brief ファイルからSceneを読み込む
	 * @param _path		ファイルパス
	 * @param _id		シーンID
	 * @return 成功したらTrue、失敗したらFalse
	 */
	bool SceneManager::LoadSceneFromFile(const std::string& _path)
	{
		return SceneSerializer::SaveScene(*ecs_, *this, _path);
	}

	/**
	 * @brief アクティブなSceneを保存する
	 * @param _path		保存先のファイルパス
	 * @return 成功: true、失敗: false
	 */
	bool SceneManager::SaveActiveScene(const std::string& _path) const
	{
		return SceneSerializer::SaveScene(*ecs_, *this, _path);
	}

	bool SceneManager::SaveSceneToFile(const std::string& _path) const
	{
		if (!active_scene_) { return false; }

		json jScene;
		const auto& entities = GetEntitiesInScene(active_scene_.value());

		for (auto e : entities) {
			jScene["entities"].push_back()
		}

	}

	/**
	 * @brief Sceneをアンロードする
	 * @param _id	シーンID
	 * @param _destroyEntities	シーンに含まれるEntityを破棄するかどうか
	 * @return 成功: true、失敗: false
	 */
	bool SceneManager::UnloadScene(Scene::Id _id, bool _destroyEntities)
	{
		auto it = scenes_.find(_id);
		if (it == scenes_.end()) { return false; }

		//// [ToDo] アンロード前処理
		//if (OnBeforeSceneUnload) {
		//	OnBeforeSceneUnload(_id);
		//}

		// Entityの破棄
		if (_destroyEntities) {
			for (auto e : it->second.entities_) {
				if (persistent_entities_.count(e)) { continue; }	// 永続化されているなら削除しない
				if (_destroyEntities) {
					ecs_->DestroyEntity(e);	// Entityの破棄
				}
			}
		}

		scenes_.erase(it);	// シーンの削除

		//// [ToDo] アンロード後処理
		//if (OnAfterSceneUnLoad) {
		//	OnAfterSceneUnLoad(_id);
		//}

		return true;
	}

	/**
	 * @brief Sceneをアクティブにする
	 * @param _id			シーンID
	 * @param unloadPrev	前のシーンをアンロードするかどうか
	 * @return 成功したらTrue、失敗したらFalse
	 */
	bool SceneManager::SetActiveScene(Scene::Id _id, bool _unloadPrev)
	{
		if (scenes_.find(_id) == scenes_.end()) { return false; } // 存在しないシーン
		if (active_scene_.has_value() && active_scene_ == _id) { return true; } // すでにアクティブ

		// 前のシーンをアンロードするなら
		if (_unloadPrev && active_scene_) {
			// アンロード
			UnloadScene(active_scene_.value());
		}

		// アクティブなシーンを切り替え
		active_scene_ = _id;

		return true;
	}

	/**
	 * @brief アクティブなSceneのIDを取得
	 * @return アクティブなSceneのID, 無い場合: nullopt
	 */
	std::optional<Scene::Id> SceneManager::GetActiveScene() const
	{
		return active_scene_;
	}

	/**
	 * @brief SceneにEntityを追加
	 * @param _id	シーンID
	 * @param _e	追加するEntity
	 */
	void SceneManager::AddEntityToScene(const Scene::Id& _id, ecs::Entity _e)
	{
		auto it = scenes_.find(_id);
		if (it == scenes_.end()) { return; } // 存在しないシーン

		it->second.entities_.push_back(_e);
	}

	/**
	 * @brief SceneからEntityを削除
	 * @param _id	シーンID
	 * @param _e	削除するEntity
	 */
	void SceneManager::RemoveEntityFromScene(const Scene::Id& _id, ecs::Entity _e)
	{
		auto it = scenes_.find(_id);
		if (it == scenes_.end()) { return; } // 存在しないシーン
		auto& ents = it->second.entities_;
		ents.erase(std::remove(ents.begin(), ents.end(), _e), ents.end());
	}

	/**
	 * @brief Sceneに含まれるEntityの一覧を取得
	 * @param _id	シーンID
	 * @return EntityのVector型リスト
	 */
	const std::vector<ecs::Entity>& SceneManager::GetEntitiesInScene(const Scene::Id& _id) const
	{
		auto it = scenes_.find(_id);
		if (it == scenes_.end()) {
			static const std::vector<ecs::Entity> empty;	// 存在しないシーン用の空リスト
			return empty;
		} // 存在しないシーン
		return it->second.entities_;
	}

	/**
	 * @brief Entityを永続化するかどうかを設定
	 * @param _e			Entity
	 * @param _persistent	永続化するかどうか
	 */
	void SceneManager::MarkPersistentEntity(ecs::Entity _e, bool _persistent)
	{
		if (_persistent) {
			persistent_entities_.insert(_e);
		}
		else {
			persistent_entities_.erase(_e);
		}
	}


	Scene::Id SceneManager::GenerateId(const std::string& _base)
	{
		std::string id = _base;
		int suffix = 1;
		while (scenes_.find(id) != scenes_.end()) {
			id = _base + "_" + std::to_string(++suffix);
		}

		return id;
	}

}
