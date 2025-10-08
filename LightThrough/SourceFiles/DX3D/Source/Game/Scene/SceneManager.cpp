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
		ImGui::Begin("Scene Editor");

		// レイアウトの計算
		const float windowWidth = ImGui::GetContentRegionAvail().x;
		const float leftW = windowWidth * 0.20f;			// Scene
		const float middleW = windowWidth * 0.40f;			// entities
		const float rightW = windowWidth - leftW - middleW;	// Inspector


		// ---------- Scene ---------- // 
		ImGui::BeginChild("ScenePane", ImVec2(leftW, 0), true);
		ImGui::Text("Scenes");
		ImGui::Separator();
		// アクティブなScene
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
				ImGui::PushID(id.c_str());
				if (ImGui::Selectable(id.c_str(), isActive)) {
					SetActiveScene(id, false);
					debug_selected_entity_.reset();
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

		if (!active_scene_) {
			ImGui::TextUnformatted("Active scene is not loaded.");
		}
		else {
			auto it = scenes_.find(*active_scene_);
			if (it == scenes_.end()) {
				ImGui::TextUnformatted("Active scene is not loaded.");
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

				if (ImGui::BeginTable("SceneEntities", 3,
					ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {
					ImGui::TableSetupColumn("Entity", ImGuiTableColumnFlags_WidthStretch);
					ImGui::TableSetupColumn("Persistent", ImGuiTableColumnFlags_WidthFixed, 70.0f);
					ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 90.0f);
					ImGui::TableHeadersRow();

					for (auto e : ents) {
						ImGui::TableNextRow();
						// Column 0: 選択されているEntity
						ImGui::TableSetColumnIndex(0);
						bool isSelected = (debug_selected_entity_ && *debug_selected_entity_ == e);
						std::string label = "Idx:" + std::to_string(e.Index()) + "Ver:" + std::to_string(e.Version());
						ImGui::PushID(static_cast<int>(e.Index()));
						if (ImGui::Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
							debug_selected_entity_ = e;
						}
						ImGui::PopID();

						// Column 1: 永続化切り替え
						ImGui::TableSetColumnIndex(1);
						bool persistent = (persistent_entities_.count(e) != 0);
						bool toggled = persistent;
						ImGui::PushID(static_cast<int>(e.Index()) + 10000);
						if (ImGui::Checkbox("##persist", &toggled)) {
							MarkPersistentEntity(e, toggled);
						}
						ImGui::PopID();

						// Column 2: State
						ImGui::TableSetColumnIndex(2);
						ImGui::TextUnformatted(e.IsInitialized() ? "Initialized" : "Uninitialized");
					}
					ImGui::EndTable();
				}
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();

		// ---------- Inspector ---------- //
		ImGui::BeginChild("InspectorPane", ImVec2(rightW, 0), true);
		ImGui::Text("Inspector");
		ImGui::Separator();

		if (!debug_selected_entity_) {
			ImGui::TextUnformatted("No entity selected.");
		}
		else {
			ecs::Entity e = *debug_selected_entity_;
			ImGui::Text("Entity Id: %u (Ver:%u)", e.Index(), e.Version());
			if (ImGui::Button("Deselect")) {
				debug_selected_entity_.reset();
			}
			ImGui::Separator();

			// ---------- Transform ---------- //
			if (ecs_.HasComponent<ecs::Transform>(e)) {
				auto& t = ecs_.GetComponent<ecs::Transform>(e);
				if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {

					// 調整速度 (Ctrlで細かく / Shiftで大きく)
					float baseSpeed = 0.05f;
					const ImGuiIO& io = ImGui::GetIO();
					if (io.KeyCtrl)  baseSpeed *= 0.2f;
					if (io.KeyShift) baseSpeed *= 4.0f;

					float pos[3] = { t.position.x, t.position.y, t.position.z };
					float rot[4] = { t.rotationQuat.x, t.rotationQuat.y, t.rotationQuat.z, t.rotationQuat.w };
					float scale[3] = { t.scale.x,     t.scale.y,     t.scale.z };

					// Position
					if (ImGui::DragFloat3("Position", pos, baseSpeed, -10000.0f, 10000.0f, "%.3f")) {
						t.SetPosition({ pos[0], pos[1], pos[2] });
					}

					// Rotation (Quaternion直接) - 正規化込み
					if (ImGui::DragFloat4("Rotation (quat)", rot, baseSpeed * 0.5f, -1.0f, 1.0f, "%.4f")) {
						float len = std::sqrt(rot[0] * rot[0] + rot[1] * rot[1] + rot[2] * rot[2] + rot[3] * rot[3]);
						if (len > 0.00001f) {
							rot[0] /= len; rot[1] /= len; rot[2] /= len; rot[3] /= len;
						}
						else {
							rot[0] = rot[1] = rot[2] = 0.0f; rot[3] = 1.0f;
						}
						t.SetRotation({ rot[0], rot[1], rot[2], rot[3] });
					}

					// Scale (負値許容するなら下限制限外す)
					if (ImGui::DragFloat3("Scale", scale, baseSpeed, -1000.0f, 1000.0f, "%.3f")) {
						t.SetScale({ scale[0], scale[1], scale[2] });
					}

					// 個別リセットボタンを横並び
					if (ImGui::BeginTable("TrButtons", 3, ImGuiTableFlags_SizingStretchSame)) {
						ImGui::TableNextColumn();
						if (ImGui::SmallButton("Reset Pos")) {
							t.SetPosition(decltype(t.position){0.0f, 0.0f, 0.0f});
						}
						ImGui::TableNextColumn();
						if (ImGui::SmallButton("Reset Rot")) {
							t.SetRotation(decltype(t.rotationQuat){0.0f, 0.0f, 0.0f, 1.0f});
						}
						ImGui::TableNextColumn();
						if (ImGui::SmallButton("Reset Scale")) {
							t.SetScale(decltype(t.scale){1.0f, 1.0f, 1.0f});
						}
						ImGui::EndTable();
					}

					ImGui::Separator();

					// 一括リセット
					if (ImGui::Button("Reset Transform")) {
						t.position = decltype(t.position){0.0f, 0.0f, 0.0f};
						t.rotationQuat = decltype(t.rotationQuat){0.0f, 0.0f, 0.0f, 1.0f};
						t.scale = decltype(t.scale){1.0f, 1.0f, 1.0f};
					}
				}
			}
			ImGui::Separator();

			// Add Component ボタン（よく使うものを列挙）
			if (ImGui::Button("Add Component")) {
				ImGui::OpenPopup("AddCompPopup");
			}
			if (ImGui::BeginPopup("AddCompPopup")) {
				if (!ecs_.HasComponent<ecs::Transform>(e) && ImGui::Selectable("Transform")) {
					ecs_.AddComponent<ecs::Transform>(e, ecs::Transform{});
				}
				if (!ecs_.HasComponent<ecs::Mesh>(e) && ImGui::Selectable("Mesh")) {
					ecs_.AddComponent<ecs::Mesh>(e, ecs::Mesh{});
				}
				if (!ecs_.HasComponent<ecs::Camera>(e) && ImGui::Selectable("Camera")) {
					ecs_.AddComponent<ecs::Camera>(e, ecs::Camera{});
				}
				if (!ecs_.HasComponent<ecs::CameraController>(e) && ImGui::Selectable("CameraController")) {
					ecs_.AddComponent<ecs::CameraController>(e, ecs::CameraController{});
				}
				ImGui::EndPopup();
			}
		}

		ImGui::EndChild();
		ImGui::End();
	}
}