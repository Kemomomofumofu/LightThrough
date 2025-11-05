/**
 * @file ForceAccumulationSystem.cpp
 * @brief AddForceやAddTorqueを反映するシステム
 */



 // ---------- インクルード ---------- // 
#include <DirectXMath.h>
#include <Game/Systems/Physics/ForceAccumulationSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Transform.h>
#include <Game/Components/Physics/Rigidbody.h>


namespace ecs {
	ForceAccumulationSystem::ForceAccumulationSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{

	}

	// 初期化
	void ForceAccumulationSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<Rigidbody>());
		ecs_.SetSystemSignature<ForceAccumulationSystem>(signature);
	}

	// 更新
	void ForceAccumulationSystem::FixedUpdate(float _fixedDt)
	{
		

		for (auto e : entities_) {
			auto& tf = ecs_.GetComponent<Transform>(e);
			auto& rb = ecs_.GetComponent<Rigidbody>(e);

			// 重力
			if (rb.useGravity) {
				rb.force.y += rb.mass * gravity_;
			}
		}
	}
}