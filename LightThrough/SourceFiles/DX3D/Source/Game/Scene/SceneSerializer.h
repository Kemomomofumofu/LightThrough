#pragma once
/**
 * @file SceneSerializer.h
 * @brief シーンのロード/セーブ
 * @author Arima Keita
 * @date 2025-09-19
 */

// ---------- インクルード ---------- //
#include <nlohmann/json.hpp>
#include <Game/Scene/Scene.h>

namespace ecs {
	class Coordinator;
	struct Entity;
}

namespace scene {
	class SceneManager;

	class SceneSerializer{
	public:
		SceneSerializer(ecs::Coordinator& _ecs);

		bool SerializeScene(const Scene& _scene, const std::string& _path);
		Scene DeserializeScene(const std::string& _path);

	private:
		nlohmann::json SerializeEntity(ecs::Coordinator& _ecs, ecs::Entity _e);
		ecs::Entity DeserializeEntity(const nlohmann::json& _j);

	private:
		ecs::Coordinator& ecs_;
	};
}