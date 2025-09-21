#pragma once
/**
 * @file SceneSerializer.h
 * @brief シーンのロード/セーブ
 * @author Arima Keita
 * @date 2025-09-19
 */

// ---------- インクルード ---------- //
#include <nlohmann/json.hpp>
#include <DX3D/Game/Scene/Scene.h>


namespace ecs {
	class Coordinator;
	struct Entity;
}

namespace scene {
	class SceneManager;

	class SceneSerializer {
	public:
		static bool SaveScene(ecs::Coordinator& _ecs, const SceneManager& _scene, const std::string& _path);

		static bool LoadScene(ecs::Coordinator& _ecs, SceneManager& _scene, const std::string& _path);

		static json SerializeEntity(ecs::Coordinator& _ecs, ecs::Entity _e);
	};
}