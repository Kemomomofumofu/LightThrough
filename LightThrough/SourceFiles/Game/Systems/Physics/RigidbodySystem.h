//#pragma once
///**
// * @file RigidbodySystem.h
// * @brief 押し出しを行うシステム
// * @author Arima Keita
// * @date 2025-10-10 4:00 有馬啓太 作成
// */
//
//
// // ---------- インクルード ---------- // 
//#include <Game/ECS/ISystem.h>
//#include <DX3D/Core/Common.h>
//
//
//namespace ecs {
//	struct Transform;
//	struct Collider;
//
//	/**
//	 * @brief 衝突時の押出処理システム
//	 * @details
//	 * - Signature: Transform, Collider
//	 */
//	class RigidbodySystem : public ISystem
//	{
//	public:
//		explicit RigidbodySystem(const SystemDesc& _desc);
//		void Init();
//		void FixedUpdate(float _fixedDt) override;
//
//
//	};
//
//}