#pragma once
/**
 * @file PrefabSystem
 * @brief オブジェクトを生成する担当
 * @author Arima Keita
 * @date 2025-08-20
 */

 // ---------- インクルード ---------- // 
#include <Game/GameObjectType.h>
#include <DX3D/Game/ECS/ISystem.h>

// ---------- 前方宣言 ---------- //
namespace ecs{
	class Coordinator;
	class Entity;
}

namespace LightThrough {

	class PrefabSystem final{
	public:
		PrefabSystem(ecs::Coordinator& _ecs) : coordinator_(_ecs) {}

		ecs::Entity CreateGameObject(LightThrough::GameObjectType _type);	// ゲームオブジェクトを生成する

	private:
		ecs::Entity CreatePlayer();
		ecs::Entity CreateCamera();

	private:
		ecs::Coordinator& coordinator_;	// コーディネーターへの参照
	};


}