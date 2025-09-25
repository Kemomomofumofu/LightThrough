/**
 * @file SceneSerializer.cpp
 * @brief SceneのSave/Load
 * @author Arima Keita
 * @date 2025-09-19
 */

 // ---------- インクルード ---------- // 
#include <fstream>
#include <DirectXMath.h>

#include <Game/ECS/Coordinator.h>
#include <Game/ECS/Entity.h>

#include <Game/Scene/SceneSerializer.h>
#include <Game/Scene/SceneManager.h>

#include <Game/Components/Transform.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>
#include <Game/Components/Mesh.h>

#define SCENE_FILE_DIR "Assets/Scenes/"



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
	 * @brief コンストラクタ
	 * @param _ecs		ECSのCoordinator
	 */
	SceneSerializer::SceneSerializer(ecs::Coordinator& _ecs)
		: ecs_(_ecs)
	{
	}

	/**
	 * @brief SceneをJSON化して保存
	 * @param _scene	対象のScene
	 * @return 成功: true, 失敗: false
	 */
	bool SceneSerializer::SerializeScene(const SceneData& _scene)
	{
		const auto path = GetSceneFilePath(_scene.name_);

		json jScene;
		jScene["sceneId"] = _scene.id_;
		jScene["sceneName"] = _scene.name_;

		// Entity一覧をJSON化
		jScene["entities"] = json::array();
		for (auto e : _scene.entities_) {
			jScene["entities"].push_back(SerializeEntity(ecs_, e));
		}

		// 書き出し
		std::ofstream ofs(path);
		if (!ofs.is_open()) {
			return false;
		}
		ofs << jScene.dump(4);	// インデント

		return true;
	}

	/**
	 * @brief JSONからSceneを復元
	 * @param _path		読み込み元のパス
	 * @return 復元したScene
	 */
	SceneData SceneSerializer::DeserializeScene(const std::string& _name)
	{
		const auto path = GetSceneFilePath(_name);

		std::ifstream ifs(path);
		if (!ifs.is_open()) {
			throw std::runtime_error("[SceneSerializer] シーンファイルを開くのに失敗" + path);
		}

		json jScene;
		ifs >> jScene;

		SceneData scene;
		scene.id_ = jScene["sceneId"].get<std::string>();
		scene.name_ = jScene["sceneName"].get<std::string>();

		for (auto& jEntity : jScene["entities"]) {
			ecs::Entity e = DeserializeEntity(jEntity);
			scene.entities_.push_back(e);
		}

		return scene;
	}

	/**
	 * @brief EntityをJSON化する
	 *
	 * コンポーネントごとに出力内容を追記する必要あり。
	 *
	 * @param _ecs		ECSのCoordinator
	 * @param _e		対象のEntity
	 * @return JSONオブジェクト
	 */
	json SceneSerializer::SerializeEntity(ecs::Coordinator& _ecs, ecs::Entity _e)
	{
		json jEntity;
		jEntity["id"] = static_cast<int>(_e.id_);

		// Transform
		if (_ecs.HasComponent<ecs::Transform>(_e)) {
			const auto& t = _ecs.GetComponent<ecs::Transform>(_e);
			jEntity["components"]["Transform"]["position"] = { t.position.x, t.position.y, t.position.z };
			jEntity["components"]["Transform"]["rotation"] = { t.rotation.x, t.rotation.y, t.rotation.z };
			jEntity["components"]["Transform"]["scale"] = { t.scale.x, t.scale.y, t.scale.z };
		}

		// Camera 
		if (_ecs.HasComponent<ecs::Camera>(_e)) {
			const auto& c = _ecs.GetComponent<ecs::Camera>(_e);
			jEntity["components"]["Camera"]["fovY"] = c.fovY;
			jEntity["components"]["Camera"]["aspectRatio"] = c.aspectRatio;
			jEntity["components"]["Camera"]["nearZ"] = c.nearZ;
			jEntity["components"]["Camera"]["farZ"] = c.farZ;
			jEntity["components"]["Camera"]["isMain"] = c.isMain;
			jEntity["components"]["Camera"]["isActive"] = c.isActive;
		}

		// CameraController
		if (_ecs.HasComponent<ecs::CameraController>(_e)) {
			const auto& cc = _ecs.GetComponent<ecs::CameraController>(_e);
			jEntity["components"]["CameraController"]["mode"] = static_cast<int>(cc.mode);
			jEntity["components"]["CameraController"]["moveSpeed"] = cc.moveSpeed;
			jEntity["components"]["CameraController"]["mouseSensitivity"] = cc.mouseSensitivity;
		}

		//// Mesh
		//if (_ecs.HasComponent<ecs::Mesh>(_e)) {
		//	const auto& m = _ecs.GetComponent<ecs::Mesh>(_e);
		//	j["components"]["Mesh"]["meshType"] = m.meshType;
		//}


		return jEntity;
	}

	/**
	 * @brief JSONからEntityを復元する
	 *
	 * コンポーネントごとに読み込み内容を追記する必要あり。
	 *
	 * @param _j		対象のJSONオブジェクト
	 * @return 復元したEntity
	 */
	ecs::Entity SceneSerializer::DeserializeEntity(const json& _j)
	{
		ecs::Entity e = ecs_.CreateEntity();

		if (!_j.contains("components") || !_j["components"].is_object()) {
			return e;
		}
		const auto& comps = _j["components"];

		// Transform
		if (comps.contains("Transform") && comps["Transform"].is_object()) {
			const auto& jt = comps["Transform"];
			if (jt.contains("position") && jt["position"].is_array() && jt["position"].size() == 3 &&
				jt.contains("rotation") && jt["rotation"].is_array() && jt["rotation"].size() == 3 &&
				jt.contains("scale") && jt["scale"].is_array() && jt["scale"].size() == 3) {

				ecs::Transform t;
				t.position = { jt["position"][0].get<float>(), jt["position"][1].get<float>(), jt["position"][2].get<float>() };
				t.rotation = { jt["rotation"][0].get<float>(), jt["rotation"][1].get<float>(), jt["rotation"][2].get<float>() };
				t.scale = { jt["scale"][0].get<float>(),    jt["scale"][1].get<float>(),    jt["scale"][2].get<float>() };
				ecs_.AddComponent(e, t);
			}
		}

		// Camera
		if (comps.contains("Camera") && comps["Camera"].is_object()) {
			const auto& jc = comps["Camera"];
			ecs::Camera c{};
			if (jc.contains("fovY"))         c.fovY = jc["fovY"].get<float>();
			if (jc.contains("aspectRatio"))  c.aspectRatio = jc["aspectRatio"].get<float>();
			if (jc.contains("nearZ"))        c.nearZ = jc["nearZ"].get<float>();
			if (jc.contains("farZ"))         c.farZ = jc["farZ"].get<float>();
			if (jc.contains("isMain"))       c.isMain = jc["isMain"].get<bool>();
			if (jc.contains("isActive"))     c.isActive = jc["isActive"].get<bool>();
			ecs_.AddComponent(e, c);
		}

		// CameraController
		if (comps.contains("CameraController") && comps["CameraController"].is_object()) {
			const auto& jcc = comps["CameraController"];
			ecs::CameraController cc{};
			if (jcc.contains("mode"))             cc.mode = static_cast<ecs::CameraMode>(jcc["mode"].get<int>());
			if (jcc.contains("moveSpeed"))        cc.moveSpeed = jcc["moveSpeed"].get<float>();
			if (jcc.contains("mouseSensitivity")) cc.mouseSensitivity = jcc["mouseSensitivity"].get<float>();
			ecs_.AddComponent(e, cc);
		}

		return e;
	}

	/**
	 * @brief シーン名からシーンファイルのパスを取得
	 * @param	_name	シーン名
	 * @return シーンファイルのパス
	 */
	std::string SceneSerializer::GetSceneFilePath(const std::string& _name)
	{
		auto path = std::string(SCENE_FILE_DIR) + _name + ".json";
		return path;
	}
}