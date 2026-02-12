/**
 * @file ClearForcesSystem.cpp
 * @brief 外力・トルクのリセット
 */

 // ---------- インクルード ---------- //
#include <Game/Systems/Physics/ClearForcesSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Components/Physics/Rigidbody.h>

namespace ecs {
	void ClearForcesSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<ecs::Rigidbody>());
		ecs_.SetSystemSignature<ClearForcesSystem>(signature);
	}


	void ClearForcesSystem::FixedUpdate(float _fixedDt)
	{
		for (auto& e : entities_) {
			auto rb = ecs_.GetComponent<ecs::Rigidbody>(e);
			// 外力・トルクリセット
			rb->force = { 0.0f, 0.0f, 0.0f };
			rb->torque = { 0.0f, 0.0f, 0.0f };
		}
	}
}