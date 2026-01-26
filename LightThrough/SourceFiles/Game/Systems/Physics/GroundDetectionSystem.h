#pragma once
/**
 * @file GroundDetectionSystem.h
 * @brief 地面接触検出システム
 * @memo GroundContactComponentの設置判定を更新するシステム
 */


// ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>

namespace ecs {
    class GroundDetectionSystem : public ISystem {
    public:
        GroundDetectionSystem(const SystemDesc& desc);
        void Init() override;
        void FixedUpdate(float fixedDt) override;

    private:
		// 地面接触情報更新
		void UpdateGround(Entity e, const DirectX::XMFLOAT3& normal);
    private:
		std::weak_ptr<class CollisionResolveSystem> collision_resolve_system_{};
    };
}
