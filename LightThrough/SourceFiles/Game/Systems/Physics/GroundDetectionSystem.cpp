/**
 * @file GroundDetectionSystem.cpp
 */

// ---------- インクルード ---------- // 
#include <Game/Systems/Physics/GroundDetectionSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Core/Transform.h>
#include <Game/Components/Physics/GroundContact.h>
#include <Game/Components/Physics/Collider.h>

#include <Game/Collisions/CollisionUtils.h>

namespace ecs {
	//! @brief コンストラクタ
    GroundDetectionSystem::GroundDetectionSystem(const SystemDesc& desc)
        : ISystem(desc)
    {
    }

	//! @brief 初期化
    void GroundDetectionSystem::Init() {
        Signature sig;
        sig.set(ecs_.GetComponentType<Transform>());
        sig.set(ecs_.GetComponentType<Collider>());
        sig.set(ecs_.GetComponentType<GroundContact>());
        ecs_.SetSystemSignature<GroundDetectionSystem>(sig);
    }

	//! @brief 固定更新
    void GroundDetectionSystem::FixedUpdate(float fixedDt) {
        for (Entity e : entities_) {
            auto& tf = ecs_.GetComponent<Transform>(e);
            auto& col = ecs_.GetComponent<Collider>(e);
            auto& gc = ecs_.GetComponent<GroundContact>(e);
            gc.isGrounded = false;
            gc.groundNormalY = -1.0f;

			// 地面判定用のレイを飛ばす

        }
    }

} // namespace ecs
