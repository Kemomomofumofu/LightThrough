#pragma once
/**
 * @file Scene.h
 * @brief シーンクラス
 * @author Arima Keita
 * @date 2025-09-15
 */

 // ---------- インクルード ---------- //
#include <string>
#include <vector>
#include <DX3D/Game/ECS/Entity.h>

namespace scene {
	/**
	 * @brief シーンクラス
	 *
	 * シーンの基本的な機能を持つクラス
	 * 
	 */
	class Scene {
	public:
		using Id = std::string;
		
		Id id_;
		std::string name_;
		std::vector<ecs::Entity> entities_{};
	};
}
