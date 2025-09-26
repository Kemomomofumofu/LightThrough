/**
 * @file SceneManager.cpp
 * @brief SceneData管理クラス
 * @author Arima Keita
 * @date 2025-09-15
 */

 // ---------- インクルード ---------- // 
#include <fstream>
#include <sstream>
#include <cassert>
#include <nlohmann/json.hpp>

#include <Game/Scene/SceneManager.h>

#include <Game/Components/Transform.h>
#include <Game/Components/Mesh.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>
//#include <Game/Components/Sprite.h>

#include <Game/GameLogUtils.h>
#include <Debug/DebugUI.h>

// JSONライブラリ
using json = nlohmann::json;


namespace scene {
	SceneManager::SceneManager(const SceneManagerDesc& _base)
		: dx3d::Base(_base.base)
		, ecs_(_base.ecs)
	{
		// SceneSerializerの生成
		serializer_ = std::make_unique<SceneSerializer>(ecs_);

		// デバッグメソッドの登録
		debug::DebugUI::ResistDebugFunction([this]() { DebugCurrentScene(); });

	}

	/**
	 * @brief SceneDataの生成
	 * @param _name		SceneData名
	 * @return 生成したSceneDataのID
	 */
	SceneData::Id SceneManager::CreateScene(const std::string& _name)
	{
		SceneData::Id id = GenerateId(_name);
		SceneData s;
		s.id_ = id;
		s.name_ = _name;
		scenes_.emplace(id, std::move(s));
		return id;
	}

	/**
	 * @brief ファイルからSceneDataを読み込む
	 * @param _path		ファイルパス
	 * @param _id		シーンID
	 * @return 成功したらTrue, 失敗したらFalse
	 */
	bool SceneManager::LoadSceneFromFile(const std::string& _name)
	{
		try {
			SceneData scene = serializer_->DeserializeScene(_name);
			auto id = scene.id_;	// moveの後で使うためキャッシュ
			scenes_.emplace(scene.id_, std::move(scene));	// シーンの追加
			active_scene_ = id;	// アクティブに
			return true;
		}
		catch (const std::exception& e) {
			GameLogFError("[SceneManager] シーンの読み込みに失敗: {} ", std::string(e.what()));
			return false;
		}
	}

	bool SceneManager::ChangeScene(const SceneData::Id& _newScene, bool _unloadPrev)
	{
		// すでにアクティブならスキップ
		if (active_scene_ && active_scene_ == _newScene) { return true; }

		auto it = scenes_.find(_newScene);
		// シーンが存在しないなら読み込み
		if (it == scenes_.end()) {
			SceneData scene = serializer_->DeserializeScene(_newScene);
			scenes_.emplace(_newScene, std::move(scene));	// シーンの追加
		}

		// 新しいSceneをアクティブに
		if (!SetActiveScene(_newScene, _unloadPrev)) { return false; }

		return true;
	}

	/**
	 * @brief アクティブなSceneDataを保存する
	 * @return 成功: true, 失敗: false
	 */
	bool SceneManager::SaveActiveScene()
	{
		if (!active_scene_) {
			GameLogError("[SceneManager] アクティブなシーンが存在しない。");
			return false;
		}

		auto it = scenes_.find(*active_scene_);
		if (it == scenes_.end()) {
			GameLogError("[SceneManager] アクティブなシーンが存在しない。");
			return false;
		}


		return serializer_->SerializeScene(it->second);
	}



	/**
	 * @brief SceneDataをアンロードする
	 * @param _id	シーンID
	 * @param _destroyEntities	シーンに含まれるEntityを破棄するかどうか
	 * @return 成功: true、失敗: false
	 */
	bool SceneManager::UnloadScene(SceneData::Id _id, bool _destroyEntities)
	{
		auto it = scenes_.find(_id);
		if (it == scenes_.end()) { return false; }

		//// [ToDo] アンロード前処理
		//if (OnBeforeSceneUnload) {
		//	OnBeforeSceneUnload(_id);
		//}

		// Entityの破棄
		// [ToDo] Scene遷移での削除する/しないをSceneで管理するべきなのかEntityで管理するべきなのか...たぶんScene側のほうが無駄なメモリは減らせるのかなぁ...
		if (_destroyEntities) {
			for (auto e : it->second.entities_) {
				if (persistent_entities_.count(e)) { continue; }	// 永続化されているなら削除しない
				ecs_.DestroyEntity(e);	// Entityの破棄
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
	 * @brief SceneDataをアクティブにする
	 * @param _id			シーンID
	 * @param unloadPrev	前のシーンをアンロードするかどうか
	 * @return 成功したらTrue、失敗したらFalse
	 */
	bool SceneManager::SetActiveScene(const SceneData::Id& _id, bool _unloadPrev)
	{
		if (scenes_.find(_id) == scenes_.end()) { return false; } // 存在しないシーン
		if (active_scene_.has_value() && active_scene_ == _id) { return true; } // すでにアクティブ

		// 前のシーンをアンロードするなら
		if (_unloadPrev && active_scene_) {
			// アンロード前処理
			if (OnBeforeSceneUnload) { OnBeforeSceneUnload(active_scene_.value()); }
			// アンロード
			UnloadScene(active_scene_.value());
		}

		// アクティブなシーンを切り替え
		active_scene_ = _id;

		// アクティブ後処理
		if (OnAfterSceneLoad) { OnAfterSceneLoad(_id); }

		return true;
	}

	/**
	 * @brief アクティブなSceneDataのIDを取得
	 * @return アクティブなSceneDataのID, 無い場合: nullopt
	 */
	std::optional<SceneData::Id> SceneManager::GetActiveScene() const
	{
		return active_scene_;
	}

	/**
	 * @brief SceneDataにEntityを追加
	 * @param _id	シーンID
	 * @param _e	追加するEntity
	 */
	void SceneManager::AddEntityToScene(const SceneData::Id& _id, ecs::Entity _e)
	{
		auto it = scenes_.find(_id);
		if (it == scenes_.end()) { return; } // 存在しないシーン

		it->second.entities_.push_back(_e);
	}

	/**
	 * @brief SceneDataからEntityを削除
	 * @param _id	シーンID
	 * @param _e	削除するEntity
	 */
	void SceneManager::RemoveEntityFromScene(const SceneData::Id& _id, ecs::Entity _e)
	{
		auto it = scenes_.find(_id);
		if (it == scenes_.end()) { return; } // 存在しないシーン
		auto& ents = it->second.entities_;
		ents.erase(std::remove(ents.begin(), ents.end(), _e), ents.end());
	}

	/**
	 * @brief SceneDataに含まれるEntityの一覧を取得
	 * @param _id	シーンID
	 * @return EntityのVector型リスト
	 */
	const std::vector<ecs::Entity>& SceneManager::GetEntitiesInScene(const SceneData::Id& _id) const
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

	/**
	 * @brief シーンIDを生成
	 * @param _base	ベースとなる名前
	 * @return ユニークなシーンID
	 */
	SceneData::Id SceneManager::GenerateId(const std::string& _base)
	{
		std::string id = _base;
		int suffix = 1;
		while (scenes_.find(id) != scenes_.end()) {
			id = _base + "_" + std::to_string(++suffix);
		}

		return id;
	}


	void SceneManager::DebugCurrentScene()
	{
		ImGui::Begin("Debug SceneManager");

		// Active Scene
		const char* activeName = "<none>";
		if (active_scene_) {
			activeName = active_scene_->c_str();
		}
		ImGui::Text("Active Scene: %s", activeName);

		// Loaded Scenes (click to activate)
		if (ImGui::CollapsingHeader("Loaded Scenes", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (auto& kv : scenes_) {
				const std::string& id = kv.first;
				bool isActive = (active_scene_ && *active_scene_ == id);
				if (ImGui::Selectable(id.c_str(), isActive)) {
					SetActiveScene(id, false);
				}
			}
		}

		// Entities in Active Scene
		if (active_scene_) {
			auto it = scenes_.find(*active_scene_);
			if (it != scenes_.end()) {
				const auto& ents = it->second.entities_;
				ImGui::Separator();
				ImGui::Text("Entity Count: %d", static_cast<int>(ents.size()));

				if (ImGui::BeginTable("SceneEntities", 3,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
				{
					ImGui::TableSetupColumn("Entity");
					ImGui::TableSetupColumn("Persistent");
					ImGui::TableSetupColumn("Details");
					ImGui::TableHeadersRow();

					for (auto e : ents) {
						ImGui::TableNextRow();

						// Column: Entity
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Idx:%u Ver:%u", e.Index(), e.Version());

						// Column: Persistent (toggle)
						ImGui::TableSetColumnIndex(1);
						bool persistent = (persistent_entities_.count(e) != 0);
						bool toggled = persistent;
						ImGui::PushID(static_cast<int>(e.Index()));
						if (ImGui::Checkbox("##persist", &toggled)) {
							MarkPersistentEntity(e, toggled);
						}
						ImGui::PopID();

						// Column: Details
						ImGui::TableSetColumnIndex(2);
						ImGui::TextUnformatted(e.IsInitialized() ? "Initialized" : "Uninitialized");
					}

					ImGui::EndTable();
				}
			}
			else {
				ImGui::TextUnformatted("Active scene is not loaded.");
			}
		}

		ImGui::End();
	}
}
