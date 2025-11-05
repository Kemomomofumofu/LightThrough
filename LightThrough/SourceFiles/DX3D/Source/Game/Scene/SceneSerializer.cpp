/**
 * @file SceneSerializer.cpp
 * @brief SceneのSave/Load
 * @author Arima Keita
 * @date 2025-09-19
 */

 // ---------- インクルード ---------- // 
#include <fstream>
#include <filesystem>
#include <DirectXMath.h>
#include <nlohmann/json.hpp>

#include <Game/ECS/Coordinator.h>
#include <Game/ECS/Entity.h>

#include <Game/Serialization/ComponentReflection.h>

#include <Game/Scene/SceneSerializer.h>
#include <Game/Scene/SceneManager.h>

#include <Game/Components/Transform.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>
#include <Game/Components/Meshrenderer.h>

#include <Debug/Debug.h>

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
} // nlohmann

namespace {
	inline std::string JsonSnippet(const json& _j, std::size_t max = 512) {
		std::string s;
		try {
			s = _j.dump(2);
			if (s.size() > max) {
				s.resize(max);
				s += "...";
			}
		}
		catch (...) {
			s = "<dump failed>";
		}
		return s;
	}
} // anonymous


namespace ecs_serial {
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
	bool SceneSerializer::SerializeScene(const scene::SceneData& _scene)
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
		for (auto& e : _scene.entities_) {
			jScene["entities"].push_back(SerializeEntity(ecs_, e));
		}

		// 書き出し
		std::ofstream ofs(path);
		if (!ofs.is_open()) {
			DebugLogError("[SceneSerializer] ファイルを開けませんでした: '{}'", path);
			return false;
		}
		ofs << jScene.dump(4);	// インデント

		DebugLogInfo("[SceneSerializer] SerializeScene done");
		return static_cast<bool>(ofs);
	}

	/**
	 * @brief JSONからSceneを復元
	 * @param _path		読み込み元のパス
	 * @return 復元したScene
	 */
	scene::SceneData SceneSerializer::DeserializeScene(const std::string& _name)
	{
		const auto path = GetSceneFilePath(_name);
		DebugLogInfo("[SceneSerializer] DeserializeScene: open '{}'", path);
		// ファイルを開く
		std::ifstream ifs(path);
		if (!ifs.is_open()) {
			throw std::runtime_error(std::string("[SceneSerializer] Fail to Open Scene File: ") + path);
		}

		// 例外処理付きでパース
		json jScene = json::parse(ifs, /*callback=*/nullptr, /*allow_exceptions=*/false);
		if (jScene.is_discarded()) {
			throw std::runtime_error(std::string("[SceneSerializer] JSONパース失敗: ") + path);
		}


		// Sceneの読み込み
		scene::SceneData scene;
		scene.id_ = jScene["sceneId"].get<std::string>();
		scene.name_ = jScene["sceneName"].get<std::string>();
		DebugLogInfo("[SceneSerializer] sceneId ='{}' sceneName = '{}'", scene.id_, scene.name_);
		// Entityの復元
		const auto& arr = jScene["entities"];
		DebugLogInfo("[SceneSerializer] entities count = '{}'", arr.size());

		size_t idx = 0;
		for (const auto& jEntity : arr) {
			DebugLogInfo("\n[SceneSerializer] DeserializeEntity index={}", idx++);
			ecs::Entity e{};
			try {
				e = DeserializeEntity(jEntity);
			}
			catch (const json::exception& _je) {
				DebugLogError("[SceneSerializer] DeserializeEntity json::exception: {} Snippet={}", _je.what(), JsonSnippet(jEntity));
				throw;
			}
			catch (const std::exception& _ex) {
				const std::string msg = _ex.what();
				DebugLogError("[SceneSerializer] DeserializeEntity std::exception: {} Snippet={}", _ex.what(), JsonSnippet(jEntity));
				throw;
			}


			scene.entities_.push_back(e);
		}

		DebugLogInfo("[SceneSerializer] DeserializeScene done");
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

		auto& registry = ecs_serial::ComponentRegistry<ecs::Coordinator, ecs::Entity>::Get();
		jEntity["components"] = registry.SerializeComponents(_ecs, _e);


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

		// Component追加
		const auto& comps = _j["components"];
		auto& registry = ecs_serial::ComponentRegistry<ecs::Coordinator, ecs::Entity>::Get();
		for (auto it = comps.begin(); it != comps.end(); ++it) {
			const std::string compName = it.key();
			const json& compData = it.value();

			DebugLogInfo("[SceneSerializer] Adding component '{}'", compName);


			bool result = registry.AddIfExists(ecs_, e, it.key(), it.value());
			if (!result) {
				DebugLogWarning("[SceneSerializer] 未登録のコンポーネント: {} をスキップ"", ", compName);
			}
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
} // scene