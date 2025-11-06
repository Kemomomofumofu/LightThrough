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


		for (auto& e : entities_) {
			auto& rb = ecs_.GetComponent<Rigidbody>(e);

			if (rb.isStatic || rb.isKinematic) { continue; }


			// 重力
			if (rb.useGravity) {
				rb.force.y += rb.mass * gravity_;
			}

			// 減衰
			// memo: 簡易モデル: (Fd = -drag * v)
			rb.force.x += -rb.drag * rb.linearVelocity.x;
			rb.force.y += -rb.drag * rb.linearVelocity.y;
			rb.force.z += -rb.drag * rb.linearVelocity.z;
			// memo: 簡易モデル: (Td = -angularDrag * ω)
			rb.torque.x += -rb.angularDrag * rb.angularVelocity.x;
			rb.torque.y += -rb.angularDrag * rb.angularVelocity.y;
			rb.torque.z += -rb.angularDrag * rb.angularVelocity.z;
		}
	}
}