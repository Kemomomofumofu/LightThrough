/**
 * @file SceneSerializer.cpp
 * @brief SceneのSave/Load
 * @author Arima Keita
 * @date 2025-09-19
 */

 // ---------- インクルード ---------- // 
#include <fstream>
#include <DirectXMath.h>

#include <DX3D/Game/ECS/Coordinator.h>
#include <DX3D/Game/ECS/Entity.h>

#include <DX3D/Game/Scene/SceneSerializer.h>
#include <DX3D/Game/Scene/SceneManager.h>

#include <Game/Components/Transform.h>
#include <Game/Components/Camera.h>
#include <Game/Components/Mesh.h>

using json = nlohmann::json;
namespace nlohmann {
	template <>
	struct adl_serializer<DirectX::XMFLOAT3> {
		static void to_json(json& _j, const DirectX::XMFLOAT3& _v) {
			_j = json{ {"x", _v.x}, {"y", _v.y}, {"z", _v.z} };
		}

		static void from_json(const json& _j, DirectX::XMFLOAT3& _v) {
			_j.at("x").get_to(_v.x);
			_j.at("y").get_to(_v.y);
			_j.at("z").get_to(_v.z);
		}
	};
}


namespace scene {

	/**
	 * @brief Sceneのセーブ
	 * @param _ecs		ECSのCoordinator
	 * @param _scene	シーンマネージャ
	 * @param _id		SceneのID
	 */
	bool SceneSerializer::SaveScene(ecs::Coordinator& _ecs, const SceneManager& _scene, const std::string& _path)
	{
		auto activeSceneId = _scene.GetActiveScene();
		if (!activeSceneId.has_value()) { return false; } // アクティブなシーンが無い

		json j;
		j["sceneId"] = activeSceneId.value();

		std::ofstream ofs(_path);
		if (!ofs.is_open()) { return false; }

		ofs << j.dump(4);
		return true;
	}

	/**
	 * @brief SceneLoad
	 * @param _ecs		ECSのCoordinator
	 * @param _scene	対象のScene
	 * @param _id		SceneのID
	 */
	bool SceneSerializer::LoadScene(ecs::Coordinator& _ecs, SceneManager& _scene, const std::string& _path)
	{
		std::ifstream ifs(_path);
		if (!ifs.is_open()) { return false; }	// 開けなかったらスキップ

		json jScene;
		ifs >> jScene;

		Scene::Id id = jScene.at("sceneId").get<Scene::Id>();
		_scene.SetActiveScene(id);

		for (const auto& jEntity : jScene["entities"]) {
			ecs::Entity e = _ecs.CreateEntity();	// Entityの生成、追加
			_scene.AddEntityToScene(id, e);	// Scene側にもEntityを追加

			const auto& comps = jEntity["components"];

			// Transform
			if (comps.contains("Transform")) {
				const auto& t = comps["Transform"];
				_ecs.AddComponent<ecs::Transform>(e, {
					{t["position"][0], t["position"][1], t["position"][2]},
					{t["rotation"][0], t["rotation"][1], t["rotation"][2]},
					{t["scale"][0], t["scale"][1], t["scale"][2]},
					});
			}

			// Camera
			if (comps.contains("Camera")) {
				const auto& c = comps["Camera"];
				_ecs.AddComponent<ecs::Camera>(e, {
					c["fovY"],
					c["aspectRatio"],
					c["nearZ"],
					c["farZ"],
					c["isMain"],
					c["isActive"],
					});
			}

			//// Mesh
			//if(comps.contains("Mesh")) {
			//	const auto& m = comps["Mesh"];
			//	_ecs.AddComponent<ecs::Mesh>(e, {
			//		m["meshType"].get<std::string>(),
			//		});
			//}
		}

		return true;
	}

	/**
	 * @brief EntityをJSON化する
	 * @param _ecs		ECSのCoordinator
	 * @param _e		対象のEntity
	 * @return JSONオブジェクト
	 */
	json SceneSerializer::SerializeEntity(ecs::Coordinator& _ecs, ecs::Entity _e)
	{
		json j;
		j["id"] = static_cast<int>(_e.id_);

		// Transform
		if (_ecs.HasComponent<ecs::Transform>(_e)) {
			const auto& t = _ecs.GetComponent<ecs::Transform>(_e);
			j["components"]["Transform"]["position"] = { t.position.x, t.position.y, t.position.z };
			j["components"]["Transform"]["rotation"] = { t.rotation.x, t.rotation.y, t.rotation.z };
			j["components"]["Transform"]["scale"] = { t.scale.x, t.scale.y, t.scale.z };
		}

		// Camera 
		if (_ecs.HasComponent<ecs::Camera>(_e)) {
			const auto& c = _ecs.GetComponent<ecs::Camera>(_e);
			j["components"]["Camera"]["fovY"] = c.fovY;
			j["components"]["Camera"]["aspectRatio"] = c.aspectRatio;
			j["components"]["Camera"]["nearZ"] = c.nearZ;
			j["components"]["Camera"]["farZ"] = c.farZ;
			j["components"]["Camera"]["isMain"] = c.isMain;
			j["components"]["Camera"]["isActive"] = c.isActive;
		}

		//// Mesh
		//if (_ecs.HasComponent<ecs::Mesh>(_e)) {
		//	const auto& m = _ecs.GetComponent<ecs::Mesh>(_e);
		//	j["components"]["Mesh"]["meshType"] = m.meshType;
		//}


		return j;
	}
}