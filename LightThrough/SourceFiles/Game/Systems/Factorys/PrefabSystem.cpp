/**
 * @file PrefabSystem.h
 * @brief オブジェクトを生成したり
 * @author Arima Keita
 * @date 2025-08-20
 */


 // ---------- インクルード ---------- // 
#include <Game/ECS/Coordinator.h>
#include <Game/ECS/Entity.h>

#include <Game/Systems/Factorys/PrefabSystem.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Velocity.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>

namespace ecs {
	Entity PrefabSystem::CreateGameObject(LightThrough::GameObjectType _type)
	{

		switch (_type) {
		case LightThrough::GameObjectType::None:
			return Entity();
			break;
		case LightThrough::GameObjectType::Camera:
			return CreateCamera();
			break;
		case LightThrough::GameObjectType::Player:
			return CreatePlayer();
			break;
		default:
			return Entity();
			break;
		}
		return Entity();
	}


	/**
	 * @brief プレイヤー生成
	 * @return プレイヤーエンティティ
	 */
	Entity PrefabSystem::CreatePlayer()
	{
		auto e = ecs_.CreateEntity();
		ecs_.AddComponent<Transform>(e, Transform{});

		return e;
	}


	/**
	 * @brief カメラ生成
	 * @return カメラエンティティ
	 */
	Entity PrefabSystem::CreateCamera()
	{
		auto e = ecs_.CreateEntity();
		ecs_.AddComponent<Transform>(e, Transform{});
		ecs_.AddComponent<Camera>(e, Camera{});
		ecs_.AddComponent<CameraController>(e, CameraController{ CameraMode::FPS });
		return e;
	}

	void PrefabSystem::Update(float _dt) {

	}
}