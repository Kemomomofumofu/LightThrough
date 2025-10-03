/**
 * @file ColliderSyncSystem.cpp
 * @brief コライダー同期システム
  * @author Arima Keita
 * @date 2025-10-03
 */


 // ---------- インクルード ---------- // 
#include <Game/Systems/Collisions/ColliderSyncSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Transform.h>
#include <Game/Components/OBB.h>
#include <Game/Components/RuntimeCollider.h>



namespace ecs {
	ColliderSyncSystem::ColliderSyncSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{

	}

	void ColliderSyncSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<OBB>());
		signature.set(ecs_.GetComponentType<RuntimeCollider>());
		ecs_.SetSystemSignature<ColliderSyncSystem>(signature);
	}


	void ColliderSyncSystem::Update(float _dt)
	{
		for (auto e : entities_) {
			auto& tf = ecs_.GetComponent<Transform>(e);
			auto& obb = ecs_.GetComponent<OBB>(e);
			auto& rtc = ecs_.GetComponent<RuntimeCollider>(e);

			// [ToDo] Transformに変更があった場合のみ更新するようにする(dirty)
			//if (obb.isStatic && !tf.dirty) { continue; }

		}
	}
}

