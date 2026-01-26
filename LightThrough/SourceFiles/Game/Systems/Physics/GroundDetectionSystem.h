#pragma once
/**
 * @file GroundDetectionSystem.h
 * @brief 地面接触検出システム
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
        float ray_distance_ = 0.12f; // レイの距離
    };
}
