#pragma once
/**
 * @file CollisionResolveSystem.h
 * @brief 押し出しを行うシステム
 */


 // ---------- インクルード ---------- // 
#include <vector>
#include <DirectXMath.h>
#include <Game/ECS/ISystem.h>
#include <DX3D/Core/Common.h>

#include <Game/Collisions/CollisionUtils.h>


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
		//! @brief 衝突情報を保存する構造体
		struct ContactRecord {
			Entity a;
			Entity b;
			collision::ContactResult contact;
			std::vector<DirectX::XMFLOAT3> samplePoints;
		};

		explicit CollisionResolveSystem(const SystemDesc& _desc);
		void Init() override;
		void FixedUpdate(float _fixedDt) override;

		//! @brief 影の中での衝突解消を有効にするか
		void SetShadowCollisionEnabled(bool _enabled) { shadow_collision_enabled_ = _enabled; }

		// 衝突リストを取得
		const std::vector<ContactRecord>& GetContacts() const { return contacts_; }
	private:
		std::weak_ptr<ShadowTestSystem> shadow_test_system_{};
		std::vector<ContactRecord> contacts_; // 衝突リスト

		float solve_percent_ = 1.0f; // 解消割合
		float solve_slop_ = 0.01f;   // 微小貫通を無視する閾値
		bool shadow_collision_enabled_ = true;	// 影での衝突解消を有効にするか
	};
}