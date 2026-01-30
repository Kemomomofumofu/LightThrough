/**
 * @file LightSpawnSystem.cpp
 * @brief ライト設置システム
 */

 // ---------- インクルード ---------- //
#include <Game/Systems/Gimmicks/LightSpawnSystem.h>

#include <Game/ECS/Coordinator.h>
#include <Game/Scene/SceneManager.h>

#include <Game/Components/GamePlay/LightPlaceRequest.h>
#include <Game/Components/Core/Transform.h>
#include <Game/Factorys/PrefabFactory.h>

namespace ecs
{
	LightSpawnSystem::LightSpawnSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{

	}

	void LightSpawnSystem::Init()
	{
		Signature sig;
		sig.set(ecs_.GetComponentType<LightPlaceRequest>());
		ecs_.SetSystemSignature<LightSpawnSystem>(sig);
	}

	void LightSpawnSystem::FixedUpdate(float _fixedDt)
	{
		for (auto& e : entities_) {
			auto req = ecs_.GetComponent<LightPlaceRequest>(e);

			// ---------- 生成 ---------- // 
			PrefabFactory prefab(ecs_);
			scene_manager_.AddEntityToScene(
				scene_manager_.GetActiveScene().value(),
				prefab.CreatePlacedLight({ req->spawnPos, req->spawnDir })
			);
			ecs_.RequestRemoveComponent<LightPlaceRequest>(e);
		}
	}

}