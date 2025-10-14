/**
 * @file CollisionResolveSystem.cpp
 * @brief 押し出しを行うシステム
 * @author Arima Keita
 * @date 2025-10-10 4:00 有馬啓太 作成
 */

 // ---------- インクルード ---------- //
#include <vector>
#include <optional>
#include <cmath>
#include <DirectXMath.h>

#include <Game/Systems/Collisions/CollisionResolveSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Transform.h>
#include <Game/Components/Collider.h>

#include <Game/Collisions/CollisionUtils.h>

namespace ecs {
	using namespace DirectX;

	namespace {
		// オーバーロード用のヘルパー
		template <class ...Ts>
		struct Overloaded : Ts... { using Ts::operator()...; };
		template <class... Ts>
		Overloaded(Ts...) -> Overloaded<Ts...>;

		/**
		 * @brief 当たり判定のディスパッチ
		 * @param _a 
		 * @param _b 
		 * @return 衝突している: ContactResult、していない: nullopt
		 */
		std::optional<collision::ContactResult>
			DispatchContact(const Collider& _a, const Collider& _b)
		{
			using collision::SphereShape;
			using collision::BoxShape;

			return std::visit(
				Overloaded{
					// Sphere vs Sphere
					[&](const SphereShape&, const SphereShape&)->std::optional<collision::ContactResult> {
						return collision::IntersectSphere(_a.worldSphere, _b.worldSphere);
					},
					// Sphere vs Box
					[&](const SphereShape&, const BoxShape&)->std::optional<collision::ContactResult> {
						return collision::IntersectSphereOBB(_a.worldSphere, _b.worldOBB);
					},
					// Box vs Sphere
					[&](const BoxShape&, const SphereShape&)->std::optional<collision::ContactResult> {
						return collision::IntersectSphereOBB(_b.worldSphere, _a.worldOBB);
					},
					// Box vs Box
					[&](const BoxShape&, const BoxShape&)->std::optional<collision::ContactResult> {
						return collision::IntersectOBB(_a.worldOBB, _b.worldOBB);
					}
				},
				_a.shape, _b.shape
			);
		}

		/**
		 * @brief 2点間の距離の二乗を計算
		 * @param _a 
		 * @param _b 
		 * @return 距離の二乗
		 */
		float DistSq(const XMFLOAT3& _a, const XMFLOAT3& _b)
		{
			float dx = _a.x - _b.x;
			float dy = _a.y - _b.y;
			float dz = _a.z - _b.z;
			return dx * dx + dy * dy + dz * dz;
		}

		/**
		 * @brief ほぼゼロベクトルかどうか
		 * @param _v 
		 * @return ほぼゼロベクトル: true, そうでない: false
		 */
		inline bool IsZeroDisp(const XMFLOAT3& _v) noexcept
		{
			return std::fabs(_v.x) + std::fabs(_v.y) + std::fabs(_v.z) < 1e-8f;
		}

	}

	


	/**
	 * @brief コンストラクタ
	 * @param _desc
	 */
	CollisionResolveSystem::CollisionResolveSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
	}

	/**
	 * @brief 初期化処理
	 */
	void CollisionResolveSystem::Init()
	{
		Signature sig;
		sig.set(ecs_.GetComponentType<Transform>());
		sig.set(ecs_.GetComponentType<Collider>());
		ecs_.SetSystemSignature<CollisionResolveSystem>(sig);
	}

	/**
	 * @brief 更新処理
	 * @param _dt 
	 */
	void CollisionResolveSystem::Update(float _dt)
	{
		// ペアごとに処理するためにベクターにコピー
		std::vector<Entity> ents;
		ents.reserve(entities_.size());
		for (const auto& e : entities_) { ents.push_back(e); };

		const size_t n = ents.size();

		for (size_t i = 0; i < n; ++i) {
			const Entity eA = ents[i];
			auto& tfA = ecs_.GetComponent<Transform>(eA);
			auto& colA = ecs_.GetComponent<Collider>(eA);
			if (colA.isStatic) { continue; } // Aがトリガーならスキップ

			for (size_t j = i + 1; j < n; ++j) {
				const Entity eB = ents[j];
				auto& tfB = ecs_.GetComponent<Transform>(eB);
				auto& colB = ecs_.GetComponent<Collider>(eB);
				if (colB.isTrigger) { continue; }	// Bがトリガーならスキップ

				// ブロードフェーズ
				const float sumR = colA.broadPhaseRadius + colB.broadPhaseRadius;
				if (DistSq(tfA.position, tfB.position) > sumR * sumR) { continue; }	// 半径の二乗より遠いならスキップ

				// ナローフェーズ
				const auto contact = DispatchContact(colA, colB);
				if (!contact) { continue; }	// 衝突していないならスキップ
				
				if (contact->penetration <= 1e-6f) { continue; } // ほとんどゼロならスキップ

				// 押し出し量の計算 (A->B 法線)
				auto [dispA, dispB] = collision::ComputePushOut(
					*contact,
					colA.isStatic, colB.isStatic,
					solvePercent_, solveSlop_);

				// 押し出し
				if (!colA.isStatic && !IsZeroDisp(dispA)) {
					tfA.AddPosition(dispA);
					tfA.BuildWorld(); // ワールド行列更新
				}
				if(!colB.isStatic && !IsZeroDisp(dispB)) {
					tfB.AddPosition(dispB);
					tfB.BuildWorld(); // ワールド行列更新
				}
			}
		}
	}

} // namespace ecs
