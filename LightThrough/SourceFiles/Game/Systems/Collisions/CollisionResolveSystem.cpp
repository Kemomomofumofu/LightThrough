/**
 * @file CollisionResolveSystem.cpp
 * @brief 押し出しを行うシステム
 */

 // ---------- インクルード ---------- //
#include <vector>
#include <optional>
#include <cmath>
#include <DirectXMath.h>

#include <Game/Systems/Collisions/CollisionResolveSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Systems/Gimmicks/ShadowTestSystem.h>

#include <Game/Components/Transform.h>
#include <Game/Components/Collider.h>
#include <Game/Components/Physics/Rigidbody.h>

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

		inline XMFLOAT3 Scale(const XMFLOAT3& _v, float _s) noexcept
		{
			return { _v.x * _s, _v.y * _s, _v.z * _s };
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

		// 影テストシステムの取得
		shadow_test_system_ = ecs_.GetSystem<ShadowTestSystem>();
	}

	/**
	 * @brief 更新処理
	 * @param _dt
	 */
	void CollisionResolveSystem::FixedUpdate(float _fixedDt)
	{
		auto shadowTestSystem = shadow_test_system_.lock();

		// ペアごとに処理するためにベクターにコピー
		std::vector<Entity> ents;
		ents.reserve(entities_.size());
		for (const auto& e : entities_) { ents.push_back(e); };

		const size_t n = ents.size();

		const float baumgarte = 0.2; // バウムガルテ係数

		for (size_t i = 0; i < n; ++i) {
			const Entity eA = ents[i];
			auto& tfA = ecs_.GetComponent<Transform>(eA);
			auto& colA = ecs_.GetComponent<Collider>(eA);

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


				// 影判定
				if (shadow_collision_enabled_ && shadowTestSystem) {
					// 衝突ペアを登録
					// memo: 次フレームの判定になる
					XMFLOAT3 contactPoint = math::Scale(math::Add(tfA.position, tfB.position), 0.5f);
					shadowTestSystem->RegisterCollisionPair(eA, eB, contactPoint);

					// 前フレームの結果を確認
					// memo: 前フレームの判定を取得する
					if (shadowTestSystem->AreBothInShadow(eA, eB)) { continue; }
				}


				// 押し出し量の計算 (A->B 法線)
				auto [dispA, dispB] = collision::ComputePushOut(
					*contact,
					colA.isStatic, colB.isStatic,
					solve_percent_, solve_slop_);

				// 押し出し
				if (!colA.isStatic && !IsZeroDisp(dispA)) {
					tfA.AddPosition(dispA);
					tfA.BuildWorld(); // ワールド行列更新
				}
				if (!colB.isStatic && !IsZeroDisp(dispB)) {
					tfB.AddPosition(dispB);
					tfB.BuildWorld(); // ワールド行列更新
				}

				// 反発
				const XMFLOAT3 nAB = contact->normal;
				float invMassA = 0.0f;
				float invMassB = 0.0f;
				Rigidbody* pRbA = nullptr;
				Rigidbody* pRbB = nullptr;

				// 質量の逆数を取得
				if (!colA.isStatic && ecs_.HasComponent<Rigidbody>(eA)) {
					auto& rbA = ecs_.GetComponent<Rigidbody>(eA);
					if (!rbA.isStatic && !rbA.isKinematic && rbA.mass > 0.0f) {
						invMassA = 1.0f / rbA.mass;
						pRbA = &rbA;
					}
				}
				if (!colB.isStatic && ecs_.HasComponent<Rigidbody>(eB)) {
					auto& rbB = ecs_.GetComponent<Rigidbody>(eB);
					if (!rbB.isStatic && !rbB.isKinematic && rbB.mass > 0.0f) {
						invMassB = 1.0f / rbB.mass;
						pRbB = &rbB;
					}
				}

				const float denom = invMassA + invMassB;
				if (denom <= 0.0f) { continue; }	// 両方0ならスキップ

				// 相対速度
				XMFLOAT3 vA{};
				XMFLOAT3 vB{};
				if (pRbA) { vA = pRbA->linearVelocity; }
				if (pRbB) { vB = pRbB->linearVelocity; }
				const XMFLOAT3 vRel = math::Sub(vB, vA);

				// 法線方向の相対速度
				const float vRelN = math::Dot(vRel, nAB);

				// 反発係数
				float e = 0.0f;
				if (pRbA) { e = (std::max)(e, std::clamp(pRbA->restitution, 0.0f, 1.0f)); }
				if (pRbB) { e = (std::max)(e, std::clamp(pRbB->restitution, 0.0f, 1.0f)); }
				// 侵入安定化バイアス(分離していても貫通があれば押し出し)
				const float penetration = (std::max)(contact->penetration - solve_slop_, 0.0f);
				const float bias = (penetration > 0.0f && _fixedDt > 0.0f) ? baumgarte * (penetration / _fixedDt) : 0.0f;

				// 法線インパルス( vRelN < 0 の時は反発も付与。分離中は反発0、バイアスのみ)
				float jn = -((vRelN < 0.0f ? (1.0f + e) * vRelN : vRelN) + bias) / denom;
				if (jn < 0.0f) { jn = 0.0f; } // 負のインパルスは無し

				const XMFLOAT3 impulseN = Scale(nAB, jn);

				// 速度適用
				if (pRbA) { pRbA->linearVelocity = math::Sub(pRbA->linearVelocity, Scale(impulseN, invMassA)); }
				if (pRbB) { pRbB->linearVelocity = math::Add(pRbB->linearVelocity, Scale(impulseN, invMassB)); }

				// クーロン摩擦の簡易モデル: (接触方向 t = normalize(vRel - vRelN * nAB))
				XMFLOAT3 t = math::Sub(vRel, Scale(nAB, vRelN));
				const float tLen = math::Length(t);
				if (tLen > 1e-6f) {
					t = Scale(t, 1.0f / tLen);
					// 合成摩擦係数 memo: ここでは平均をとる
					float mu = 0.0f;
					if (pRbA) { mu += std::clamp(pRbA->friction, 0.0f, 1.0f); }
					if (pRbB) { mu += std::clamp(pRbB->friction, 0.0f, 1.0f); }
					mu = (pRbA && pRbB) ? (mu * 0.5f) : mu;

					// 接触インパルス
					float jt = -(math::Dot(vRel, t) / denom);
					// クランプ ( |jt| <= mu * jn )
					const float jtMax = mu * jn;
					jt = std::clamp(jt, -jtMax, jtMax);

					const XMFLOAT3 impulseT = Scale(t, jt);
					if (pRbA) { pRbA->linearVelocity = math::Sub(pRbA->linearVelocity, Scale(impulseT, invMassA)); }
					if (pRbB) { pRbB->linearVelocity = math::Add(pRbB->linearVelocity, Scale(impulseT, invMassB)); }
				}
			}
		}
	}
} // namespace ecs
