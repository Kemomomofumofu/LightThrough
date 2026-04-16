/**
 * @file SceneManager.cpp
 * @brief SceneData管理クラス
 * @author Arima Keita
 * @date 2025-09-15
 */

 // ---------- インクルード ---------- // 
#include <limits>
#include <fstream>
#include <sstream>
#include <cassert>
#include <nlohmann/json.hpp>

#include <Game/Scene/SceneManager.h>

#include <Game/ECS/Coordinator.h>
#include <Game/Components/Core/Transform.h>
#include <Game/Components/Render/MeshRenderer.h>
#include <Game/Components/Render/SpriteRenderer.h>
#include <Game/Components/Camera/Camera.h>
#include <Game/Components/Input/CameraController.h>
#include <Game/Components/Physics/Collider.h>
#include <Game/Components/Physics/Rigidbody.h>
#include <Game/Components/Physics/GroundContact.h>
#include <Game/Components/Render/Light.h>
#include <Game/Components/Input/PlayerController.h>
#include <Game/Components/Input/MoveDirectionSource.h>
#include <Game/Components/Core/Name.h>
#include <Game/Components/Core/ObjectRoot.h>
#include <Game/Components/Core/ObjectChild.h>
#include <Game/Components/GamePlay/LightPlaceRequest.h>
#include <Game/Components/Events/TriggerEvents.h>

#include <Game/ECS/ECSUtils.h>
#include <Game/GameLogUtils.h>
#include <DX3D/Math/MathUtils.h>
#include <Debug/DebugUI.h>

// JSONライブラリ
using json = nlohmann::json;
using namespace DirectX;

namespace scene {
	//! @brief コンストラクタ
	SceneManager::SceneManager(const SceneManagerDesc& _base)
		: dx3d::Base(_base.base)
		, ecs_(_base.ecs)
	{
		// SceneSerializerの生成
		serializer_ = std::make_unique<ecs_serial::SceneSerializer>(ecs_);

		// デバッグメソッドの登録
		debug::DebugUI::ResistDebugFunction([this]() { DebugCurrentScene(); });
	}

	//! @brief シーン生成
	SceneData::Id SceneManager::CreateScene(const std::string& _name)
	{
		SceneData::Id id = GenerateId(_name);
		SceneData s;
		s.id_ = id;
		s.name_ = _name;
		scenes_.emplace(id, std::move(s));
		return id;
	}

	//! @brief シーンをファイルから読み込む
	bool SceneManager::LoadSceneFromFile(const std::string& _name)
	{
		try {
			SceneData scene = serializer_->DeserializeScene(_name);
			auto& id = scene.id_;	// moveの後で使うためキャッシュ
			scenes_.emplace(id, std::move(scene));	// シーンの追加
			return true;
		}
		catch (const std::exception& e) {
			GameLogFError("[SceneManager] シーンの読み込みに失敗: {} ", std::string(e.what()));
			return false;
		}
	}

	//! @brief シーンをプリロードする
	bool SceneManager::PreloadScene(const std::string& _name)
	{
		// すでに読み込み済み
		if (preloaded_scenes_.find(_name) != preloaded_scenes_.end()) {
			DebugLogError("[SceneManager] すでにプリロードされたシーン: {}", _name);
			return false;
		}
		// すでに存在
		if (scenes_.find(_name) != scenes_.end()) {
			DebugLogError("[SceneManager] すでに存在するシーン: {}", _name);
			return false;
		}


		try {
			SceneData scene = serializer_->DeserializeScene(_name);
			preloaded_scenes_.emplace(_name, std::move(scene));
			return true;
		}
		catch (const std::exception& _e) {
			DebugLogError("[SceneManager] シーンのプリロードに失敗: {}", std::string(_e.what()));
			return false;
		}
	}

	//! @brief プリロードされたシーンをアクティブにする
	bool SceneManager::ActivatePreloadedScene(const std::string& _name)
	{
		auto it = preloaded_scenes_.find(_name);
		if (it == preloaded_scenes_.end()) {
			DebugLogError("[SceneManager] プリロードされたシーンが見つからない: {}", _name);
			return false;
		}

		auto& id = it->second.id_;
		scenes_.emplace(id, std::move(it->second));
		preloaded_scenes_.erase(it);
		return true;
	}

	//! @brief シーン切り替え
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


	//! @brief シーン切り替えリクエスト
	void SceneManager::RequestChangeScene(const SceneData::Id& _newScene)
	{
		if (pending_scene_change_) {
			DebugLogWarning("[SceneManager] 保留中のシーン切り替えを上書き {} -> {}", *pending_scene_change_, _newScene);
		}
		pending_scene_change_ = _newScene;
		DebugLogInfo("[SceneManager] シーン切り替えリクエスト: {}", _newScene);
	}
	//! @brief シーン切り替えリクエスト（遷移情報付き）
	void SceneManager::RequestChangeScene(const SceneData::Id& _newScene, const SceneTransitionInfo& _info)
	{
		pending_transition_info_ = _info;
		RequestChangeScene(_newScene);
	}

	//! @brief 保留中のシーン切り替えリクエストを実行
	bool SceneManager::FlushSceneChangeRequest()
	{
		if (!pending_scene_change_) { return false; }

		SceneData::Id newScene = std::move(*pending_scene_change_);
		pending_scene_change_.reset();

		return ChangeScene(newScene);
	}

	//! @brief シーンの追加
	bool SceneManager::AddScene(const SceneData::Id& _id)
	{
		if (scenes_.find(_id) != scenes_.end()) { return false; }
		try {
			SceneData scene = serializer_->DeserializeScene(_id);
			scenes_.emplace(_id, std::move(scene));
			return true;
		}
		catch (const std::exception& _e) {
			DebugLogError("[SceneManager] シーンの追加に失敗: {}", std::string(_e.what()));
			return false;
		}
	}


	//! @brief アクティブなSceneDataをリロードする
	bool SceneManager::ReloadActiveScene()
	{
		const auto& id = *active_scene_;
		// アンロード
		if (!UnloadScene(id)) {
			GameLogFError("[SceneManager] シーンのアンロードに失敗: {}", id);
			return false;
		}

		// ロード
		if (!LoadSceneFromFile(id)) {
			GameLogFError("[SceneManager] シーンのロードに失敗: {}", id);
			return false;
		}

		// システムにシーンロード通知
		for (auto& system : ecs_.GetAllSystems()) {
			system->OnSceneLoaded();
		}

		return true;
	}

	//! @brief 全てのシーンをリロードする
	bool SceneManager::ReloadAllScene()
	{
		// 走査中にコンテナが変更されるため、先にIDを全て収集する
		std::vector<SceneData::Id> ids;
		ids.reserve(scenes_.size());
		for (const auto& [id, _] : scenes_) {
			ids.push_back(id);
		}

		// 現在のアクティブシーンを保持
		auto& prevActive = active_scene_;

		for (const auto& id : ids) {
			// アンロード
			if (!UnloadScene(id)) {
				GameLogFError("[SceneManager] シーンのアンロードに失敗: {}", id);
				return false;
			}
			// ロード
			if (!LoadSceneFromFile(id)) {
				GameLogFError("[SceneManager] シーンのロードに失敗: {}", id);
				return false;
			}
		}

		// アクティブシーンを復元
		if (prevActive) {
			active_scene_ = *prevActive;
		}

		// システムにシーンロード通知
		for (auto& system : ecs_.GetAllSystems()) {
			system->OnSceneLoaded();
		}

		return true;
	}


	//! @brief アクティブなSceneDataを保存する
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

	//! @brief Sceneの保存
	bool SceneManager::SaveScene(const SceneData::Id& _id)
	{
		auto it = scenes_.find(_id);
		if (it == scenes_.end()) {
			DebugLogError("[SceneManager] シーンが存在しない: {}", _id);
			return false;
		}

		return serializer_->SerializeScene(it->second);
	}



	//! @brief SceneDataのアンロード
	bool SceneManager::UnloadScene(SceneData::Id _id, bool _destroyEntities)
	{
		auto it = scenes_.find(_id);
		if (it == scenes_.end()) { return false; }

		//// [ToDo] アンロード前処理
		//if (OnBeforeSceneUnload) {
		//	OnBeforeSceneUnload(_id);
		//}

		// Entityの破棄
		if (_destroyEntities) {
			// memo: 走査中に要素数が変わるので、一度別コンテナに対比してから破棄する。
			std::vector<ecs::Entity> toDestroy;
			toDestroy.reserve(it->second.entities_.size());

			for (auto& e : it->second.entities_) {
				toDestroy.push_back(e);
			}

			for (auto& e : toDestroy) {
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

	//! @brief アクティブなシーンを設定
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

		// システムにシーンロード通知
		for (auto& system : ecs_.GetAllSystems()) {
			system->OnSceneLoaded();
		}

		// アクティブ後処理
		if (OnAfterSceneLoad) { OnAfterSceneLoad(_id); }

		return true;
	}

	//! @brief アクティブなシーンIDを取得
	std::optional<SceneData::Id> SceneManager::GetActiveScene() const
	{
		return active_scene_;
	}

	//! @brief SceneDataにEntityを追加
	void SceneManager::AddEntityToScene(const SceneData::Id& _id, ecs::Entity _e)
	{
		auto it = scenes_.find(_id);
		if (it == scenes_.end()) { return; } // 存在しないシーン

		it->second.entities_.push_back(_e);
	}

	//! @brief SceneDataからEntityを削除
	void SceneManager::RemoveEntityFromScene(const SceneData::Id& _id, ecs::Entity _e)
	{
		auto it = scenes_.find(_id);
		if (it == scenes_.end()) { return; } // 存在しないシーン
		auto& ents = it->second.entities_;
		ents.erase(std::remove(ents.begin(), ents.end(), _e), ents.end());
	}

	//! @brief SceneDataに含まれるEntity一覧を取得
	const std::vector<ecs::Entity>& SceneManager::GetEntitiesInScene(const SceneData::Id& _id) const
	{
		auto it = scenes_.find(_id);
		if (it == scenes_.end()) {
			static const std::vector<ecs::Entity> empty;	// 存在しないシーン用の空リスト
			return empty;
		} // 存在しないシーン
		return it->second.entities_;
	}


	//! @brief Entity破棄時コールバック
	void SceneManager::OnEntityDestroyed(ecs::Entity _e)
	{
		// すべてのシーンから削除
		for (auto& kv : scenes_) {
			auto& ents = kv.second.entities_;
			ents.erase(std::remove(ents.begin(), ents.end(), _e), ents.end());
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
		ImGui::Begin("Scene Editor");

		// レイアウトの計算
		const float windowWidth = ImGui::GetContentRegionAvail().x;
		const float leftW = windowWidth * 0.2f;			// Scene
		const float middleW = windowWidth * 0.4f;			// entities
		const float rightW = windowWidth - leftW - middleW;	// Inspector


		// ---------- Scene ---------- // 
		// LoadScene
		{
			ImGui::BeginChild("ScenePane", ImVec2(leftW, 0), true);

			ImGui::Text("Scene Ops");
			ImGui::Separator();

			// シーン名入力
			char buf[256]{};
			std::snprintf(buf, sizeof(buf), "%s", debug_load_name_input_.c_str());
			if (ImGui::InputText("LoadName", buf, sizeof(buf))) {
				debug_load_name_input_ = buf;
			}

			// シーンロードボタン
			if (ImGui::Button("Load Scene")) {
				if (!debug_load_name_input_.empty()) {
					bool ok = LoadSceneFromFile(debug_load_name_input_);
					if (ok) {
						debug_selected_scene_ = *active_scene_; // ロードしたシーンを選択状態にする
						debug_selected_entity_.reset();
					}
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload Active Scene")) {
				ReloadActiveScene();
				debug_selected_entity_.reset();
			}
		}

		// Create Scene
		{
			char buf[256]{};
			std::snprintf(buf, sizeof(buf), "%s", debug_scene_name_input_.c_str());
			if (ImGui::InputText("NewSceneName", buf, sizeof(buf))) {
				debug_scene_name_input_ = buf;
			}

			if (ImGui::Button("Create Scene")) {
				if (!debug_scene_name_input_.empty()) {
					auto id = CreateScene(debug_scene_name_input_);
					debug_selected_scene_ = id;
				}
			}
		}

		ImGui::Spacing();
		ImGui::Separator();

		// Selected Scene ops
		SceneData::Id targetScene;
		bool hasTarget = false;

		if (debug_selected_scene_) {
			targetScene = *debug_selected_scene_;
			hasTarget = true;
		}

		if (hasTarget) {
			ImGui::Text("Target: %s", targetScene.c_str());

			if (ImGui::Button("Set Active")) {
				SetActiveScene(targetScene, false);
				debug_selected_entity_.reset();
			}
			ImGui::SameLine();
			if (ImGui::Button("Save")) {
				SaveScene(targetScene);
			}
			ImGui::SameLine();
			if (ImGui::Button("Unload")) {
				ImGui::OpenPopup("ConfirmUnloadScene");
			}

			if (ImGui::BeginPopupModal("ConfirmUnloadScene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Unload scene '%s' ?", targetScene.c_str());
				ImGui::Separator();

				if (ImGui::Button("OK")) {
					if (active_scene_ && *active_scene_ == targetScene) {
						DebugLogError("[SceneManager] アクティブなシーンはアンロードできません\n");
					}
					else {
						UnloadScene(targetScene, true);
					}
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
		else {
			ImGui::TextDisabled("No target scene.");
		}

		ImGui::Text("Scenes");
		ImGui::Separator();
		// 選択されているScene
		const char* activeName = "<none>";
		if (active_scene_) {
			activeName = active_scene_->c_str();
		}
		ImGui::Text("Active: %s", activeName);
		// ロードされているScene
		if (ImGui::CollapsingHeader("Loaded Scenes", ImGuiTreeNodeFlags_DefaultOpen)) {
			for (auto& kv : scenes_) {
				const std::string& id = kv.first;

				bool isActive = (active_scene_ && *active_scene_ == id);
				bool isSelected = (debug_selected_scene_ && *debug_selected_scene_ == id);

				ImGui::PushID(id.c_str());

				// 選択の見た目を isSelected にしたいなら第2引数を isSelected にする
				if (ImGui::Selectable(id.c_str(), isSelected)) {
					debug_selected_scene_ = id;
					debug_selected_entity_.reset();
				}

				// Activeにする操作は別ボタンに分ける（誤爆防止）
				if (isSelected) {
					ImGui::SameLine();
					if (ImGui::SmallButton("Activate")) {
						SetActiveScene(id, false);
						debug_selected_entity_.reset();
					}
				}

				// Active表示
				if (isActive) {
					ImGui::SameLine();
					ImGui::TextDisabled("(Active)");
				}

				ImGui::PopID();
			}
		}

		ImGui::EndChild();
		ImGui::SameLine();

		// ---------- Entity ---------- // 
		ImGui::BeginChild("EntitiesPana", ImVec2(middleW, 0), true);
		ImGui::Text("Entities");
		ImGui::Separator();

		// エンティティ生成
		if (ImGui::Button("Create Entity")) {
			ecs::Entity e = ecs_.CreateEntity();

			// 最低限のコンポーネントを付ける
			ecs_.AddComponent<ecs::Transform>(e, {});
			ecs::Name nameComp;
			nameComp.value = "Entity_" + std::to_string(e.Index());
			ecs_.AddComponent<ecs::Name>(e, nameComp);

			// Scene に登録
			if (debug_selected_scene_) {
				AddEntityToScene(*debug_selected_scene_, e);
			}
		}


		if (!debug_selected_scene_) {
			ImGui::TextUnformatted("No scene selected.");
		}
		else {
			auto it = scenes_.find(*debug_selected_scene_);
			if (it == scenes_.end()) {
				ImGui::TextUnformatted("Selected scene is not loaded.");
			}
			else {
				auto& ents = it->second.entities_;
				// 選択されているEntityが存在しないならリセット
				if (debug_selected_entity_) {
					bool found = std::find(ents.begin(), ents.end(), *debug_selected_entity_) != ents.end();
					if (!found) { debug_selected_entity_.reset(); }
				}

				ImGui::Text("Entity Count: %d", static_cast<int>(ents.size()));
				ImGui::Spacing();

				if (ImGui::BeginTable("SceneEntities", 1,
					ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {
					ImGui::TableSetupColumn("Entity", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableHeadersRow();

					for (auto& e : ents) {
						ImGui::TableNextRow();
						// Column 0: 選択されているEntity
						ImGui::TableSetColumnIndex(0);
						bool isSelected = (debug_selected_entity_ && *debug_selected_entity_ == e);
						// 表示名
						std::string label;
						if (ecs_.HasComponent<ecs::Name>(e)) {
							label = ecs_.GetComponent<ecs::Name>(e)->value;
						}
						else {
							label = "Idx:" + std::to_string(e.Index()) + "Ver:" + std::to_string(e.Version());
						}

						ImGui::PushID(static_cast<int>(e.Index()));
						if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
							debug_selected_entity_ = e;
						}
						ImGui::PopID();
					}
					ImGui::EndTable();
				}
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();

		// ---------- Inspector ---------- //
		ImGui::BeginChild("InspectorPane", ImVec2(0, 0), true);
		if (debug_selected_entity_) {
			ecs::Entity e = *debug_selected_entity_;

			// Entity削除ボタン
			if (ImGui::Button("Delete Entity")) {
				ecs_.RequestDestroyEntity(e);
				debug_selected_entity_.reset();
				ImGui::EndChild();
				ImGui::End();
				return;
			}
			ImGui::Separator();

			ImGui::Text("Entity Id: %u (Ver:%u)", e.Index(), e.Version());
			if (ImGui::Button("Deselect")) {
				debug_selected_entity_.reset();
			}
			ImGui::Separator();

			float baseSpeed = 0.05f;
			const ImGuiIO& io = ImGui::GetIO();
			if (io.KeyCtrl)  baseSpeed *= 0.2f;
			if (io.KeyShift) baseSpeed *= 4.0f;

			auto& registry = ecs_serial::ComponentRegistry::Get();
			for (const auto& [name, entry] : registry.GetAllEntries()) {
				if (!entry.has || !entry.has(ecs_, e)) { continue; }

				ImGui::PushID(name.c_str());
				if (ImGui::CollapsingHeader(name.c_str())) {
					// Inspector描画
					if (entry.inspect) {
						entry.inspect(ecs_, e, baseSpeed);
					}
					// コンポーネント削除ボタン
					if (entry.remove && ImGui::Button("Remove Component")) {
						entry.remove(ecs_, e);
					}
				}
				ImGui::PopID();
				ImGui::Separator();
			}

			// Add Component Popup
			if (ImGui::Button("Add Component")) {
				ImGui::OpenPopup("AddCompPopup");
			}
			if (ImGui::BeginPopup("AddCompPopup")) {
				for (const auto& [name, entry] : registry.GetAllEntries()) {
					if (entry.has && entry.has(ecs_, e)) { continue; }
					if (ImGui::Selectable(name.c_str())) {
						registry.AddIfExists(ecs_, e, name, nlohmann::json::object());
					}
				}
				ImGui::EndPopup();
			}
		}
		else {
			ImGui::TextUnformatted("No entity selected.");
		}

		ImGui::EndChild();
		ImGui::End();
	}
}