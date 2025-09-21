#pragma once
/**
 * @file SceneData.h
 * @brief どのエンティティがシーンに属しているかなどを管理するクラス
 * @author Arima Keita
 * @date 2025-09-15
 */

 // ---------- インクルード ---------- //
#include <string>
#include <vector>
#include <Game/ECS/Entity.h>

namespace scene {
	/**
	 * @brief シーンクラス
	 *
	 * Scene固有のID, 名前, 含まれるEntityの一覧を管理する
	 * SceneManagerによって管理される
	 * 
	 */
	class SceneData {
	public:
		using Id = std::string;
		
		Id id_;
		std::string name_;
		std::vector<ecs::Entity> entities_{};
	};
}
