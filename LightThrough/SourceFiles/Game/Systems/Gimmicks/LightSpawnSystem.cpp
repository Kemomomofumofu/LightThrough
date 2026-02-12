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
#include <Game/Components/Input/PlayerController.h>
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
		sig.set(ecs_.GetComponentType<PlayerController>());
		sig.set(ecs_.GetComponentType<LightPlaceRequest>());
		ecs_.SetSystemSignature<LightSpawnSystem>(sig);
	}

	void LightSpawnSystem::FixedUpdate(float _fixedDt)
	{
		for (auto& e : entities_) {
			auto req = ecs_.GetComponent<LightPlaceRequest>(e);
			auto pc = ecs_.GetComponent<PlayerController>(e);

			// 上限チェック
			if(pc->ownedLights.size() >= pc->maxOwnedLights) {
				// 古いライトを削除
				Entity oldLight = pc->ownedLights.front();
				pc->ownedLights.pop_front();
				ecs_.RequestDestroyEntity(oldLight);
			}

			// 生成
			PrefabFactory prefab(ecs_);
			Entity lightEntity = prefab.CreatePlacedLight({ req->spawnPos, req->spawnDir });

			// シーンに追加
			scene_manager_.AddEntityToScene(
				scene_manager_.GetActiveScene().value(),
				lightEntity
			);

			// 生成したEntityにライトを所持させる
			pc->ownedLights.push_back(lightEntity);

			// リクエストコンポーネント削除
			ecs_.RequestRemoveComponent<LightPlaceRequest>(e);
		}
	}

}