#pragma once
/**
 * @file ColliderSyncSystem.h
 * @brief コライダーの同期システム
 * @author Arima Keita
 * @date 2025-10-03
 */



 // ---------- インクルード ---------- // 
#include <Game/ECS/ISystem.h>

namespace ecs {
	struct Transform;
	struct Collider;

	/**
	 * @brief コライダーの同期システム
	 * @details Transformコンポーネントの変更を検知して、Colliderコンポーネントの形状を更新する
	 */
	class ColliderSyncSystem : public ISystem {
	public:
		explicit ColliderSyncSystem(const SystemDesc& _desc);

		void Init();
		void FixedUpdate(float _fixedDt) override;

	private:
		void BuildSphere(const Transform& _tf, Collider& _col);
		void BuildOBB(const Transform& _tf, Collider& _com);
	};
}