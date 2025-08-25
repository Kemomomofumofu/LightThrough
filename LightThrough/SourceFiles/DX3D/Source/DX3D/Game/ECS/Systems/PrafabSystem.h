#pragma once
/**
 * @file PrefabSystem
 * @brief オブジェクトを生成する担当
 * @author Arima Keita
 * @date 2025-08-20
 */

 // ---------- インクルード ---------- // 
#include <DX3D/Game/LightThrough/GameObjectType.h>

// ---------- 前方宣言 ---------- //
namespace ecs{
	class Coordinator;
	class Entity;
}

namespace LightThrough {

	class PrefabSystem final {
	public:
		PrefabSystem(ecs::Coordinator& _coord) : coordinator_(_coord) {}

		ecs::Entity CreateGameObject(LightThrough::GameObjectType _type);	// ゲームオブジェクトを生成する

	private:
		ecs::Entity CreatePlayer();
		ecs::Entity CreateCamera();

	private:
		ecs::Coordinator& coordinator_;	// コーディネーターへの参照
	};


}