#pragma once
/**
 * @file CollisionResolveSystem.h
 * @brief 押し出しを行うシステム
 * @author Arima Keita
 * @date 2025-10-10 4:00 有馬啓太 作成
 */


 // ---------- インクルード ---------- // 
#include <Game/ECS/ISystem.h>
#include <DX3D/Core/Common.h>


namespace ecs {
	struct Transform;
	struct Collider;

	/**
	 * @brief 衝突時の押出処理システム
	 * @details
	 * - Signature: Transform, Collider
	 */
	class CollisionResolveSystem : public ISystem
	{
	public:
		explicit CollisionResolveSystem(const SystemDesc& _desc);
		void Init();
		void FixedUpdate(float _fixedDt) override;

	private:
		float solvePercent_ = 0.8f; // 解消割合
		float solveSlop_ = 0.01f;   // 微小貫通を無視する閾値
	};

}