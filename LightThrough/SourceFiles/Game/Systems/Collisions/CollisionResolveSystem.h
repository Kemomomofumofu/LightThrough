#pragma once
/**
 * @file CollisionResolveSystem.h
 * @brief 押し出しを行うシステム
 */


 // ---------- インクルード ---------- // 
#include <Game/ECS/ISystem.h>
#include <DX3D/Core/Common.h>


namespace ecs {
	struct Transform;
	struct Collider;
	class ShadowTestSystem;

	/**
	 * @brief 衝突時の押出処理システム
	 * @details
	 * - Signature: Transform, Collider
	 */
	class CollisionResolveSystem : public ISystem
	{
	public:
		explicit CollisionResolveSystem(const SystemDesc& _desc);
		void Init() override;
		void FixedUpdate(float _fixedDt) override;

		//! @brief 影の中での衝突解消を有効にするか
		void SetShadowCollisionEnabled(bool _enabled) { shadow_collision_enabled_ = _enabled; }

	private:
		std::weak_ptr<ShadowTestSystem> shadow_test_system_{};

		float solve_percent_ = 1.0f; // 解消割合
		float solve_slop_ = 0.01f;   // 微小貫通を無視する閾値
		bool shadow_collision_enabled_ = true;	// 影での衝突解消を有効にするか
	};
}