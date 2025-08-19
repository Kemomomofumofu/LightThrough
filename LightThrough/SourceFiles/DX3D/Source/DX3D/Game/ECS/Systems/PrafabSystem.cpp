/**
 * @file PrefabSystem.h
 * @brief オブジェクトを生成したり
 * @author Arima Keita
 * @date 2025-08-20
 */


 // ---------- インクルード ---------- // 
#include <DX3D/Game/ECS/Systems/PrafabSystem.h>
#include <DX3D/Game/ECS/Coordinator.h>
#include <DX3D/Game/ECS/Entity.h>

#include <DX3D/Game/ECS/Components/Transform.h>
#include <DX3D/Game/ECS/Components/Velocity.h>

namespace LightThrough {


	ecs::Entity PrefabSystem::CreateGameObject(LightThrough::GameObjectType _type)
	{

		switch (_type) {
		case GameObjectType::None:
			return ecs::Entity();
			break;
		case GameObjectType::Camera:
			return CreateCamera();
			break;
		case GameObjectType::Player:
			return CreatePlayer();
			break;
		default:
			return ecs::Entity();
			break;
		}
		return ecs::Entity();
	}


	/**
	 * @brief プレイヤー生成
	 * @return プレイヤーエンティティ
	 */
	ecs::Entity PrefabSystem::CreatePlayer()
	{
		auto e = coordinator_.CreateEntity();
		coordinator_.AddComponent<ecs::Transform>(e, ecs::Transform());
		coordinator_.AddComponent<ecs::Velocity>(e, ecs::Velocity{});
		return e;
	}


	/**
	 * @brief カメラ生成
	 * @return カメラエンティティ
	 */
	ecs::Entity PrefabSystem::CreateCamera()
	{
		auto e = coordinator_.CreateEntity();
		coordinator_.AddComponent<ecs::Transform>(e, ecs::Transform{});
		return e;
	}
}