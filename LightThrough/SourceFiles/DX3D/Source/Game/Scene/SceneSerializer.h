#pragma once
/**
 * @file SceneSerializer.h
 * @brief シーンのロード/セーブ
 * @author Arima Keita
 * @date 2025-09-19
 */

// ---------- インクルード ---------- //
#include <nlohmann/json.hpp>
#include <Game/Scene/SceneData.h>

namespace ecs {
	class Coordinator;
	struct Entity;
}

namespace ecs_serial {
	class SceneManager;
	/**
	 * @brief シーンのロード/セーブを行うクラス
	 */
	class SceneSerializer{
	public:
		SceneSerializer(ecs::Coordinator& _ecs);

		bool SerializeScene(const scene::SceneData& _scene);
		scene::SceneData DeserializeScene(const std::string& _path);

	private:
		nlohmann::json SerializeEntity(ecs::Coordinator& _ecs, ecs::Entity _e);
		ecs::Entity DeserializeEntity(const nlohmann::json& _j);

		std::string GetSceneFilePath(const std::string& _name);

	private:
		ecs::Coordinator& ecs_;
	};
}