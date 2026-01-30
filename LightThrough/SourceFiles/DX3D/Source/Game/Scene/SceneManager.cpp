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

#include <Game/ECS/ECSUtils.h>
#include <Game/GameLogUtils.h>
#include <Debug/DebugUI.h>

// JSONライブラリ
using json = nlohmann::json;
using namespace DirectX;
// ---------------- 追加ヘルパ ---------------- //
namespace {

	template<class T>
	bool DrawValueWidget(const char* _label, T& _value, float _speed = 0.05f)
	{
		// bool
		if constexpr (std::is_same_v<T, bool>) {
			return ImGui::Checkbox(_label, &_value);
		}
		// 浮動小数
		else if constexpr (std::is_same_v<T, float>) {
			return ImGui::DragFloat(_label, &_value, _speed);
		}
		// 整数
		else if constexpr (std::is_same_v<T, int>) {
			return ImGui::DragInt(_label, &_value, static_cast<int>(_speed * 10.0f));
		}
		else if constexpr (std::is_same_v<T, uint32_t>) {
			int tmp = static_cast<int>(_value);
			bool ch = ImGui::DragInt(_label, &tmp, static_cast<int>(_speed * 10.0f), 0);
			if (ch) { _value = static_cast<uint32_t>((std::max)(tmp, 0)); }
			return ch;
		}
		// 文字列
		else if constexpr (std::is_same_v<T, std::string>) {
			char buf[256];
			std::snprintf(buf, sizeof(buf), "%s", _value.c_str());
			if (ImGui::InputText(_label, buf, sizeof(buf))) {
				_value = buf;
				return true;
			}
			return false;
		}
		// enum (整数として編集)
		else if constexpr (std::is_enum_v<T>) {
			using UT = std::underlying_type_t<T>;
			int tmp = static_cast<int>(static_cast<UT>(_value));
			bool ch = ImGui::DragInt(_label, &tmp, 1, (std::numeric_limits<int>::min)(), (std::numeric_limits<int>::max)());
			if (ch) { _value = static_cast<T>(static_cast<UT>(tmp)); }
			return ch;
		}
		// Vec3Like
		else if constexpr (ecs_serial::Vec3Like<T>) {
			float arr[3]{ _value.x, _value.y, _value.z };
			if (ImGui::DragFloat3(_label, arr, _speed)) {
				_value.x = arr[0]; _value.y = arr[1]; _value.z = arr[2];
				return true;
			}
			return false;
		}
		// Vec4Like
		else if constexpr (ecs_serial::Vec4Like<T>) {
			float arr[4]{ _value.x, _value.y, _value.z, _value.w };
			if (ImGui::DragFloat4(_label, arr, _speed)) {
				_value.x = arr[0]; _value.y = arr[1]; _value.z = arr[2]; _value.w = arr[3];
				return true;
			}
			return false;
		}
		// 未対応
		else {
			ImGui::TextDisabled("%s (unsupported type)", _label);
			return false;
		}
	}

	template<class Comp>
	void DrawReflectedComponentFields(Comp& comp, float speed)
	{
		auto fields = ecs_serial::TypeReflection<Comp>::Fields();
		ecs_serial::for_each(fields, [&](auto&& fieldInfo) {
			auto& member = comp.*(fieldInfo.member);
			const char* fname = fieldInfo.name.data();
			DrawValueWidget(fname, member, speed);
			});
	}

	template<>
	void DrawReflectedComponentFields<ecs::Transform>(ecs::Transform& _tf, float _speed)
	{
		// position
		{
			XMFLOAT3 tmp = _tf.position;
			if (DrawValueWidget("Position", tmp, _speed)) {
				_tf.SetPosition(tmp);
			}
		}
		// scale
		{
			XMFLOAT3 tmp = _tf.scale;
			if (DrawValueWidget("Scale", tmp, _speed)) {
				_tf.SetScale(tmp);
			}
		}
		// rotation (quat + euler)
		{
			XMFLOAT4 qtmp = _tf.rotationQuat;
			if (DrawValueWidget("Rotation (Quat)", qtmp, _speed * 0.5f)) {
				XMVECTOR q = XMVectorSet(qtmp.x, qtmp.y, qtmp.z, qtmp.w);
				q = XMQuaternionNormalize(q);
				XMFLOAT4 norm; XMStoreFloat4(&norm, q);
				_tf.SetRotation(norm);
			}
			_tf.SyncEulerFromQuat();
			auto euler = _tf.GetRotationEulerDeg();
			float deg[3]{ euler.x, euler.y, euler.z };
			if (ImGui::DragFloat3("Rotation (deg)", deg, 0.5f)) {
				_tf.SetRotationEulerDeg({ deg[0], deg[1], deg[2] });
			}
		}

		ImGui::Separator();
		// 方向ベクトル確認表示
		{
			auto& f = _tf.GetForward();
			auto& r = _tf.GetRight();
			auto& u = _tf.GetUp();
			ImGui::Text("Fwd:(%.2f %.2f %.2f)", f.x, f.y, f.z);
			ImGui::Text("Right:(%.2f %.2f %.2f)", r.x, r.y, r.z);
			ImGui::Text("Up:(%.2f %.2f %.2f)", u.x, u.y, u.z);
		}
	}

	template<>
	void DrawReflectedComponentFields<ecs::Collider>(ecs::Collider& c, float speed)
	{
		const char* shapeItems[] = { "Sphere", "Box" };
		int current = static_cast<int>(c.type);

		if (ImGui::Combo("ShapeType", &current, shapeItems, IM_ARRAYSIZE(shapeItems))) {
			c.type = static_cast<collision::ShapeType>(current);
			c.shapeDirty = true;
		}

		switch (c.type)
		{
		case collision::ShapeType::Sphere:
		{
			if (ImGui::DragFloat("Radius", &c.sphere.radius, speed, 0.01f)) {
				c.shapeDirty = true;
			}
			break;
		}
		case collision::ShapeType::Box:
		{
			if (ImGui::DragFloat3("HalfExtents", &c.box.halfExtents.x, speed)) {
				c.shapeDirty = true;
			}
			break;
		}
		default:
			break;
		}

		ImGui::Checkbox("IsTrigger", &c.isTrigger);
		ImGui::Checkbox("IsStatic", &c.isStatic);
	}

} // unnamed namespace

namespace scene {
	SceneManager::SceneManager(const SceneManagerDesc& _base)
		: dx3d::Base(_base.base)
		, ecs_(_base.ecs)
	{
		// SceneSerializerの生成
		serializer_ = std::make_unique<ecs_serial::SceneSerializer>(ecs_);

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
			auto& id = scene.id_;	// moveの後で使うためキャッシュ
			active_scene_ = id;	// アクティブに
			scenes_.emplace(id, std::move(scene));	// シーンの追加
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

	//! @brief アクティブなSceneDataをリロードする
	bool SceneManager::ReloadActiveScene()
	{
		const auto id = *active_scene_;
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
			for (auto& e : it->second.entities_) {
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
		const float leftW = windowWidth * 0.2f;			// Scene
		const float middleW = windowWidth * 0.4f;			// entities
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

		// エンティティ生成
		if (ImGui::Button("Create Entity")) {
			ecs::Entity e = ecs_.CreateEntity();

			// 最低限のコンポーネントを付ける
			ecs_.AddComponent<ecs::Transform>(e, {});
			ecs::Name nameComp;
			nameComp.value = "Entity_" + std::to_string(e.Index());
			ecs_.AddComponent<ecs::Name>(e, nameComp);

			// アクティブ Scene に登録
			if (active_scene_) {
				AddEntityToScene(*active_scene_, e);
			}
		}


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
		std::optional<ecs::ComponentType> removeComponentType;

		ImGui::BeginChild("InspectorPane", ImVec2(rightW, 0), true);
		ImGui::Text("Inspector");
		ImGui::Separator();

		if (!debug_selected_entity_) {
			ImGui::TextUnformatted("No entity selected.");
		}
		else {
			ecs::Entity e = *debug_selected_entity_;

			// Entity削除ボタン
			if (ImGui::Button("Delete Entity")) {
				// Entityの削除待機リストに追加する
				ecs_.RequestDestroyEntity(e);

				debug_selected_entity_.reset();
				ImGui::EndChild();
				ImGui::End();
				return;
			}
			ImGui::Separator();

			// Entity情報
			ImGui::Text("Entity Id: %u (Ver:%u)", e.Index(), e.Version());
			if (ImGui::Button("Deselect")) {
				debug_selected_entity_.reset();
			}
			ImGui::Separator();

			// 調整速度 (Ctrl / Shift)
			float baseSpeed = 0.05f;
			const ImGuiIO& io = ImGui::GetIO();
			if (io.KeyCtrl)  baseSpeed *= 0.2f;
			if (io.KeyShift) baseSpeed *= 4.0f;

			// ここで列挙したい全コンポーネント型タプル
			using AllComponents = std::tuple<
				ecs::Name,
				ecs::Transform,
				ecs::GroundContact,
				ecs::MeshRenderer,
				ecs::Camera,
				ecs::CameraController,
				ecs::PlayerController,
				ecs::MoveDirectionSource,
				ecs::Collider,
				ecs::Rigidbody,
				ecs::LightCommon,
				ecs::SpotLight,
				ecs::ObjectRoot,
				ecs::ObjectChild,
				ecs::LightPlaceRequest
			>;

			// メタループ
			ecs_serial::for_each(AllComponents{}, [&](auto&& dummyComp) {
				using CompT = std::decay_t<decltype(dummyComp)>;
				if (ecs_.HasComponent<CompT>(e)) {
					auto* compPtr = ecs_.GetComponent<CompT>(e);
					const char* compName = ecs_serial::TypeReflection<CompT>::Name().data();
					ImGui::PushID(compName);
					if (ImGui::CollapsingHeader(compName)) {
						auto& compRef = *compPtr;
						DrawReflectedComponentFields(compRef, baseSpeed);
						// コンポーネント削除ボタン
						if (ImGui::Button("Remove Component")) {
							removeComponentType = ecs_.GetComponentType<CompT>();
							// コンポーネント削除待機リストに追加
							ecs_.RequestRemoveComponent<CompT>(e);
						}
					}
					ImGui::PopID();
					ImGui::Separator();
				}
				});

			// Add Component Popup
			if (ImGui::Button("Add Component")) {
				ImGui::OpenPopup("AddCompPopup");
			}
			if (ImGui::BeginPopup("AddCompPopup")) {
				auto& registry = ecs_serial::ComponentRegistry::Get();
				for (const auto& [name, entry] : registry.GetAllEntries()) {
					// すでに持っているコンポーネントはスキップ
					if (entry.has && entry.has(ecs_, e)) { continue; }

					if (ImGui::Selectable(name.c_str())) {
						// コンポーネント追加待機リストに追加
						bool result = registry.AddIfExists(ecs_, e, name, nlohmann::json::object());
						if (!result) {
							DebugLogError("[SceneManager] コンポーネントの追加に失敗: {}", name);
						}
					}
				}
				ImGui::EndPopup();
			}
		}

		ImGui::EndChild();
		ImGui::End();
	}
}