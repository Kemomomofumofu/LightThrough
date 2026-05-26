#pragma once
/**
 * @file CollisionResolveSystem.h
 * @brief 押し出しを行うシステム
 */


 // ---------- インクルード ---------- // 
#include <vector>
#include <unordered_set>
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
			bool shadowSkiped = false; // 影判定でスキップされたか
		};

		//! @brief トリガー衝突記録
		struct TriggerRecord {
			Entity a;
			Entity b;
		};

		explicit CollisionResolveSystem(const SystemDesc& _desc);
		void Init() override;
		void FixedUpdate(float _fixedDt) override;
		void OnSceneLoaded() override;

		//! @brief 影の中での衝突解消を有効にするか
		void SetShadowCollisionEnabled(bool _enabled) { shadow_collision_enabled_ = _enabled; }

		//! @brief 衝突リストを取得
		const std::vector<ContactRecord>& GetContactRecords() const { return contact_records_; }
		//! @brief トリガー衝突リストを取得
		const std::vector<TriggerRecord>& GetTriggerRecords() const { return trigger_records_; }
	private:
		using EntityPair = std::pair<Entity, Entity>;
		struct EntityPairHash {
			std::size_t operator()(const EntityPair& _p) const {
				return std::hash<Entity>{}(_p.first) ^ (std::hash<Entity>{}(_p.second) << 1);
			}
		};

		void CollectCollisionPairs(std::unordered_set<std::pair<Entity, Entity>, EntityPairHash>& _currentContacts);
		void NormalizeContacts();
		void RegisterShadowTestPoints();
		void ProcessTriggerEvents();
		void ShadowFilterCollisions();
		void SolvePenetration();
		void SolveVelocity(float _dt);
		void UpdateShadowSkipPairs();


	private:
		std::weak_ptr<ShadowTestSystem> shadow_test_system_{};
		std::vector<ContactRecord> contact_records_{}; // 衝突リスト
		std::vector<TriggerRecord> trigger_records_{}; // トリガー衝突リスト



		std::unordered_set<std::pair<Entity, Entity>, EntityPairHash> shadow_skip_pairs_{}; // 影で衝突解消をスキップするペア集合

		float solve_percent_ = 2.0f; // 解消割合
		float solve_slop_ = 0.01f;   // 微小貫通を無視する閾値
		bool shadow_collision_enabled_ = true;	// 影での衝突解消を有効にするか

		float time_ = 0.0f; // 時間計測用
	};
}