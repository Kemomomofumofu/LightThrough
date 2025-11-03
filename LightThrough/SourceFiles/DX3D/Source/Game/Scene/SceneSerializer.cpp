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

#include <Game/Serialization/ComponentReflection.h>

#include <Game/Scene/SceneSerializer.h>
#include <Game/Scene/SceneManager.h>

#include <Game/Components/Transform.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>
#include <Game/Components/Meshrenderer.h>

constexpr auto SCENE_FILE_DIR = "Assets/Scenes/";



using json = nlohmann::json;
namespace nlohmann {
	/**
	 * @brief DirectX::XMFLOAT3 のJSON変換
	 */
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

	/**
	 * @brief DirectX::XMFLOAT4 のJSON変換
	 */
	template <>
	struct adl_serializer<DirectX::XMFLOAT4> {
		static void to_json(json& _j, const DirectX::XMFLOAT4& _v) {
			_j = json{ {"x", _v.x}, {"y", _v.y}, {"z", _v.z}, {"w", _v.w} };
		}
		static void from_json(const json& _j, DirectX::XMFLOAT4& _v) {
			_j.at("x").get_to(_v.x);
			_j.at("y").get_to(_v.y);
			_j.at("z").get_to(_v.z);
			_j.at("w").get_to(_v.w);
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

		// 保存ディレクトリの確保
		std::error_code ec;
		std::filesystem::create_directories(SCENE_FILE_DIR, ec);


		json jScene;
		jScene["sceneId"] = _scene.id_;
		jScene["sceneName"] = _scene.name_;
		jScene["version"] = 1;	// [ToDo] バージョン管理用

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

		return static_cast<bool>(ofs);
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

		// 例外処理付きでパース
		json jScene = json::parse(ifs, /*callback=*/nullptr, /*allow_exceptions=*/false);
		if (jScene.is_discarded()) {
			throw std::runtime_error(std::string("[SceneSerializer] JSONパース失敗: ") + path);
		}

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
			jEntity["components"]["Transform"]["rotationQuat"] = { t.rotationQuat.x, t.rotationQuat.y, t.rotationQuat.z, t.rotationQuat.w };
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

		// Mesh
		if (_ecs.HasComponent<ecs::MeshRenderer>(_e)) {
			const auto& m = _ecs.GetComponent<ecs::MeshRenderer>(_e);
			jEntity["components"]["MeshRenderer"]["handle"] = m.handle.id;
		}


		return jEntity;
	}

	/**
	 * @brief JSONからEntityを復元する
	 * @param _j	対象のJSONオブジェクト
	 * @return 復元したEntity
	 */
	ecs::Entity SceneSerializer::DeserializeEntity(const json& _j)
	{
		// Entity生成
		ecs::Entity e = ecs_.CreateEntity();
		// Componentsがなければ終了
		if (!_j.contains("components") || !_j["components"].is_object()) { return e; }

		const auto& comps = _j["components"];
		// ComponentRegistryの取得
		auto& registry = ecs_serial::ComponentRegistry<ecs::Coordinator, ecs::Entity>::Get();
		for (auto it = comps.begin(); it != comps.end(); ++it) {
			// 存在するComponentなら追加
			// 未登録なら何もしない [ToDo] Logに出せたらよいかも。
			registry.AddIfExists(ecs_, e, it.key(), it.value());
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