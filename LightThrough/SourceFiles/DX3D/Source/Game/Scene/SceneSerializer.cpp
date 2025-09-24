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
	 * @param _path		保存先のパス
	 * @return 成功: true, 失敗: false
	 */
	bool SceneSerializer::SerializeScene(const SceneData& _scene, const std::string& _name)
	{
		auto& path = GetSceneFilePath(_name);

		json jScene;
		jScene["sceneId"] = _scene.id_;
		jScene["sceneName"] = _scene.name_;

		// Entity一覧をJSON化
		jScene["entities"] = json::array();
		for(auto e : _scene.entities_) {
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
		auto& path = GetSceneFilePath(_name);

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

		// Transform
		if(_j["components"].contains("Transform")) {
			ecs::Transform t;
			auto& jt = _j["Transform"];
			t.position = { jt["position"][0], jt["position"][1], jt["position"][2] };
			t.rotation = { jt["rotation"][0], jt["rotation"][1], jt["rotation"][2] };
			t.scale = { jt["scale"][0], jt["scale"][1], jt["scale"][2] };

			ecs_.AddComponent(e, t);
		}

		// Camera
		if(_j["components"].contains("Camera")) {
			ecs::Camera c;
			auto& jc = _j["Camera"];
			c.fovY = jc["fovY"];
			c.aspectRatio = jc["aspectRatio"];
			c.nearZ = jc["nearZ"];
			c.farZ = jc["farZ"];
			c.isMain = jc["isMain"];
			c.isActive = jc["isActive"];
			ecs_.AddComponent(e, c);
		}

		// CameraController
		if (_j["components"].contains("CameraController")) {
			ecs::CameraController cc;
			auto& jcc = _j["CameraController"];
			cc.mode = static_cast<ecs::CameraMode>(jcc["mode"].get<int>());
			cc.moveSpeed = jcc["moveSpeed"];
			cc.mouseSensitivity = jcc["mouseSensitivity"];
			ecs_.AddComponent(e, cc);
		}

		return e;
	}

	/**
	 * @brief シーン名からシーンファイルのパスを取得
	 * @param _sceneName	シーン名
	 * @return シーンファイルのパス
	 */
	std::string& SceneSerializer::GetSceneFilePath(const std::string& _sceneName)
	{
		constexpr const char* basePath = "Assets/Scenes/";
		auto path = std::string(basePath) + _sceneName + ".json";
		return path;
	}
}