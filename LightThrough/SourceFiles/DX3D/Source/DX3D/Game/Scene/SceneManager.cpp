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
#include <DX3D/Game/Scene/SceneManager.h>
//#include <nlohmann/json.hpp>

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


		// ---------- デシリアライザ ---------- // 
		// Transform
		RegisterComponentDeserializer("Transform", [](ecs::Coordinator& _ecs, ecs::Entity _e, const std::string& _jsonText) {
			auto j = json::parse(_jsonText);

			DirectX::XMFLOAT3 position{
				j["position"].value("x", 0.0f),
				j["position"].value("y", 0.0f),
				j["position"].value("z", 0.0f),
			};
			DirectX::XMFLOAT3 rotation{
				j["rotation"].value("x", 0.0f),
				j["rotation"].value("y", 0.0f),
				j["rotation"].value("z", 0.0f),
			};
			DirectX::XMFLOAT3 scale{
				j["scale"].value("x", 1.0f),
				j["scale"].value("y", 1.0f),
				j["scale"].value("z", 1.0f),
			};
			ecs::Transform t{ position, rotation, scale };
			_ecs.AddComponent(_e, t);
			});

		// Camera
		RegisterComponentDeserializer("Camera", [](ecs::Coordinator& _ecs, ecs::Entity _e, const std::string& _jsonText) {
			auto j = json::parse(_jsonText);
			ecs::Camera c{};
			c.fovY = j.value("fovY", 45.0f);
			c.aspectRatio = j.value("aspectRatio", 16.0f / 9.0f);
			c.nearZ = j.value("nearZ", 0.1f);
			c.farZ = j.value("farZ", 100.0f);

			c.isMain = j.value("isMain", false);
			c.isActive = j.value("isActive", true);
			_ecs.AddComponent(_e, c);
			});

		// CameraController
		RegisterComponentDeserializer("CameraController", [](ecs::Coordinator& _ecs, ecs::Entity _e, const std::string& _jsonText) {
			auto j = json::parse(_jsonText);
			ecs::CameraController cc{};

			cc.mode = j.value("mode", ecs::CameraMode::Orbit);
			cc.moveSpeed = j.value("moveSpeed", 5.0f);
			cc.mouseSensitivity = j.value("mouseSensitivity", 1.0f);

			cc.yaw = j.value("yaw", 0.0f);
			cc.pitch = j.value("pitch", 0.0f);

			if (j.contains("orbitTarget") && j["orbitTarget"].is_array() && j["orbitTarget"].size() == 3) {
				cc.orbitTarget.x = j["orbitTarget"][0].get<float>();
				cc.orbitTarget.y = j["orbitTarget"][1].get<float>();
				cc.orbitTarget.z = j["orbitTarget"][2].get<float>();
			}
			cc.orbitDistance = j.value("orbitDistance", 5.0f);
			cc.orbitYaw = j.value("orbitYaw", 0.0f);
			cc.orbitPitch = j.value("orbitPitch", 0.3f);

			cc.invertY = j.value("invertY", false);

			_ecs.AddComponent(_e, cc);
			});

		// Sprite [ToDo] まだ仮置き
		RegisterComponentDeserializer("Sprite", [](ecs::Coordinator& _ecs, ecs::Entity _e, const std::string& _jsonText) {
			auto j = json::parse(_jsonText);
			// ecs::Sprite s{ j["path"]};
			//_ecs.AddComponent(_e, s);
			});




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
	 * @return 成功したらtrue、失敗したらfalse
	 */
	bool SceneManager::LoadSceneFromFile(const std::string& _path, Scene::Id _id)
	{
		if (ecs_ == nullptr) { return false; } // 初期化されていない

		std::ifstream ifs(_path);
		if (!ifs.is_open()) { return false; }

		json j;
		try {
			ifs >> j;
		}
		catch (...) {
			return false;
		}

		std::string sceneName = j.value("sceneName", _id.empty() ? "scene" : _id);
		Scene::Id sceneId = _id.empty() ? GenerateId(sceneName) : _id;

		if (OnBeforeSceneLoad) {
			OnBeforeSceneLoad(sceneId);
		}

		Scene scene;
		scene.id_ = sceneId;
		scene.name_ = sceneName;

		// エンティティ
		if (j.contains("entities") && j["entities"].is_array()) {
			for (auto& entDef : j["entities"]) {
				ecs::Entity e = ecs_->CreateEntity();// Entityの生成

				// コンポーネントがあるなら
				if (entDef.contains("components") && entDef["components"].is_object()) {
					for (auto it = entDef["components"].begin(); it != entDef["components"].end(); ++it) {
						std::string compName = it.key();
						std::string compText = it.value().dump();

						auto deserIt = component_deserializers_.find(compName);
						if (deserIt != component_deserializers_.end()) {
							deserIt->second(*ecs_, e, compText); // デシリアライズ関数を呼び出す
						}
						else {
							// 未登録のコンポーネント
							assert(false && "Component deserializer not found");
						}
					}
				}
				scene.entities_.push_back(e);
			}
		}


		scenes_.emplace(sceneId, std::move(scene));

		if (OnAfterSceneLoad) {
			OnAfterSceneLoad(sceneId);
		}

		return true;
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

		// アンロード前処理
		if (OnBeforeSceneUnload) {
			OnBeforeSceneUnload(_id);
		}

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

		// アンロード後処理
		if (OnAfterSceneUnLoad) {
			OnAfterSceneUnLoad(_id);
		}

		return true;
	}

	/**
	 * @brief Sceneをファイルに保存する
	 * @param _id		シーンID
	 * @param _path		ファイルパス
	 * @return 成功したらtrue、失敗したらfalse
	 */
	bool SceneManager::SaveSceneToFile(const Scene::Id& _id, const std::string& _path)
	{
		// [ToDo] 実装 プロジェクトのシリアライズ方針次第
		return false;
	}

	/**
	 * @brief Sceneをアクティブにする
	 * @param _id			シーンID
	 * @param unloadPrev	前のシーンをアンロードするかどうか
	 * @return 成功したらtrue、失敗したらfalse
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

	void SceneManager::RegisterComponentDeserializer(const std::string& _componentName, ComponentDeserializer _deserializer)
	{
		component_deserializers_[_componentName] = std::move(_deserializer);
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
