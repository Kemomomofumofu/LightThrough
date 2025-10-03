#pragma once
/**
 * @file ColliderSyncSystem.h
 * @brief コライダーの同期システム
 * @author Arima Keita
 * @date 2025-10-03
 */



 // ---------- インクルード ---------- // 
#include <Game/ECS/ISystem.h>
#include <Game/Components/Transform.h>
#include <Game/Components/OBB.h>
#include <Game/Components/RuntimeCollider.h>

namespace ecs {
	class ColliderSyncSystem : public ISystem {
	public:
		explicit ColliderSyncSystem(const SystemDesc& _desc);

		void Init();
		void Update(float _dt) override;

	private:
		uint64_t current_frame_{}; // 現在のフレーム
	};
}