#pragma once
/**
 * @file PrefabSystem
 * @brief オブジェクトを生成する担当
 * @author Arima Keita
 * @date 2025-08-20
 */

// ---------- インクルード ---------- // 
#include <DX3D/Game/LightThrough/GameObjectType.h>

namespace LightThrough {
	class Coordinator;
	class Entity;

	class PrefabSystem final {
	public:
		PrefabSystem(Coordinator& _coord);

		Entity CreateGameObject(LightThrough::GameObjectType _type);

	private:
		Entity CreatePlayer();
		Entity CreateCamera();

	private:
		ecs::Coordinator& coordinator_;	// コーディネーターへの参照
	};


}