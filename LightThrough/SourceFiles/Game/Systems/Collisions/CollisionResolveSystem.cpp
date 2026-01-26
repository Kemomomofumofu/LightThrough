/**
 * @file CollisionResolveSystem.cpp
 * @brief 押し出しを行うシステム
 */

 // ---------- インクルード ---------- //
#include <vector>
#include <optional>
#include <cmath>
#include <variant>
#include <DirectXMath.h>

#include <Game/Systems/Collisions/CollisionResolveSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Systems/Gimmicks/ShadowTestSystem.h>

#include <Game/Components/Core/Transform.h>
#include <Game/Components/Physics/Collider.h>
#include <Game/Components/Physics/Rigidbody.h>
#include <Game/Components/Physics/GroundContact.h>
#include <Game/Components/Core/Name.h>

#include <Debug/Debug.h>

namespace ecs {
	using namespace DirectX;

	namespace {
		template <class ...Ts>
		struct Overloaded : Ts... { using Ts::operator()...; };
		template <class... Ts>
		Overloaded(Ts...) -> Overloaded<Ts...>;

		inline bool IsZeroDisp(const XMFLOAT3& _v) noexcept
		{
			return std::fabs(_v.x) + std::fabs(_v.y) + std::fabs(_v.z) < 1e-8f;
		}

		inline XMFLOAT3 Scale(const XMFLOAT3& _v, float _s) noexcept
		{
			return { _v.x * _s, _v.y * _s, _v.z * _s };
		}

		std::optional<collision::ContactResult>
			DispatchContact(const Collider& _a, const Collider& _b)
		{
			using collision::SphereShape;
			using collision::BoxShape;


			return std::visit(
				Overloaded{
				[&](const SphereShape&, const SphereShape&)->std::optional<collision::ContactResult> {
				return collision::IntersectSphere(_a.worldSphere, _b.worldSphere);
				},
				[&](const SphereShape&, const BoxShape&)->std::optional<collision::ContactResult> {
				return collision::IntersectSphereOBB(_a.worldSphere, _b.worldOBB);
				},
				[&](const BoxShape&, const SphereShape&)->std::optional<collision::ContactResult> {
				return collision::IntersectSphereOBB(_b.worldSphere, _a.worldOBB);
				},
				[&](const BoxShape&, const BoxShape&)->std::optional<collision::ContactResult> {
				return collision::IntersectOBB(_a.worldOBB, _b.worldOBB);
				}
				},
				_a.shape, _b.shape
			);
		}

		// Contact を一時保存する構造体
		struct ContactRecord {
			Entity a;
			Entity b;
			collision::ContactResult contact;
			std::vector<XMFLOAT3> samplePoints; // Shadow 判定用のサンプル点
			bool skipDueToShadow = false; // ExecuteShadowTests 実行後に設定
		};

		/**
		 * @brief 衝突面上のサンプルポイントを生成
		 * @param _center 衝突面の中心（コライダー表面）
		 * @param _normal 衝突法線
		 * @param _radius サンプル範囲の半径
		 * @param _outPoints 出力先
		 * @param _gridSize グリッドサイズ（3なら3x3=9点）
		 */
		void GenerateSurfaceSamplePoints(
			const XMFLOAT3& _center,
			const XMFLOAT3& _normal,
			float _radius,
			std::vector<XMFLOAT3>& _outPoints,
			int _gridSize = 3)
		{
			// 法線に垂直な2つの軸を生成
			XMFLOAT3 tangent1{}, tangent2{};
			if (std::fabs(_normal.y) < 0.99f) {
				XMFLOAT3 up{ 0, 1, 0 };
				tangent1 = math::Cross(_normal, up);
			}
			else {
				XMFLOAT3 right{ 1, 0, 0 };
				tangent1 = math::Cross(_normal, right);
			}
			tangent1 = math::Normalize(tangent1);
			tangent2 = math::Normalize(math::Cross(_normal, tangent1));

			// グリッド状にサンプリング
			float step = (_gridSize > 1) ? (2.0f * _radius / (_gridSize - 1)) : 0.0f;
			float startOffset = -_radius;

			for (int i = 0; i < _gridSize; ++i) {
				for (int j = 0; j < _gridSize; ++j) {
					float offsetU = (_gridSize > 1) ? (startOffset + step * i) : 0.0f;
					float offsetV = (_gridSize > 1) ? (startOffset + step * j) : 0.0f;

					XMFLOAT3 samplePoint{
						_center.x + tangent1.x * offsetU + tangent2.x * offsetV,
						_center.y + tangent1.y * offsetU + tangent2.y * offsetV,
						_center.z + tangent1.z * offsetU + tangent2.z * offsetV
					};
					_outPoints.push_back(samplePoint);
				}
			}
		}

		/** * @brief エンティティのサンプル半径を取得 */
		float GetSampleRadius(const Collider& _col)
		{
			// Sphere の場合
			if (_col.type == collision::ShapeType::Sphere) {
				return _col.worldSphere.radius * 0.5f;
			}
			// Box の場合
			else {
				float minHalf = (std::min)({ _col.worldOBB.half.x, _col.worldOBB.half.y, _col.worldOBB.half.z });
				return minHalf * 0.8f;
			}
		}


	} // namespace anonymous

	CollisionResolveSystem::CollisionResolveSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
	}

	//! @brief 初期化
	void CollisionResolveSystem::Init()
	{
		Signature sig;
		sig.set(ecs_.GetComponentType<Transform>());
		sig.set(ecs_.GetComponentType<Collider>());
		ecs_.SetSystemSignature<CollisionResolveSystem>(sig);

		shadow_test_system_ = ecs_.GetSystem<ShadowTestSystem>();
	}

	//! @brief 固定更新
	void CollisionResolveSystem::FixedUpdate(float _fixedDt)
	{
		auto shadowTestSystem = shadow_test_system_.lock();

		// 処理対象をローカルにコピー
		std::vector<Entity> ents;
		ents.reserve(entities_.size());
		for (const auto& e : entities_) { ents.push_back(e); }

		const size_t n = ents.size();
		const float baumgarte = 0.2f;

		// 接触情報の収集
		contacts_.clear();
		for (size_t i = 0; i < n; ++i) {
			const Entity eA = ents[i];
			auto& tfA = ecs_.GetComponent<Transform>(eA);
			auto& colA = ecs_.GetComponent<Collider>(eA);

			for (size_t j = i + 1; j < n; ++j) {
				const Entity eB = ents[j];
				auto& tfB = ecs_.GetComponent<Transform>(eB);
				auto& colB = ecs_.GetComponent<Collider>(eB);

				if (colB.isTrigger) { continue; }

				// ブロードフェーズ
				const float sumR = colA.broadPhaseRadius + colB.broadPhaseRadius;
				auto DistSq = [](const XMFLOAT3& a, const XMFLOAT3& b) {
					float dx = a.x - b.x; float dy = a.y - b.y; float dz = a.z - b.z;
					return dx * dx + dy * dy + dz * dz;
					};
				if (DistSq(tfA.position, tfB.position) > sumR * sumR) { continue; }

				// ナローフェーズ
				auto contactOpt = DispatchContact(colA, colB);
				if (!contactOpt) { continue; }
				if (contactOpt->penetration <= 1e-6f) { continue; }

				// 接触情報を蓄積
				ContactRecord r;
				r.a = eA;
				r.b = eB;
				r.contact = *contactOpt;

				// サンプルポイントの生成
				// 小さいほうを基準にする
				const Collider* targetCol = nullptr;
				XMFLOAT3 surfaceNormal = math::Normalize(r.contact.normal);

				// Box vs Box
				if (colA.type == collision::ShapeType::Box && colB.type == collision::ShapeType::Box)
				{
					float volA =
						colA.worldOBB.half.x *
						colA.worldOBB.half.y *
						colA.worldOBB.half.z;
					float volB =
						colB.worldOBB.half.x *
						colB.worldOBB.half.y *
						colB.worldOBB.half.z;

					if (volA <= volB) {
						targetCol = &colA;
					}
					else {
						surfaceNormal = Scale(surfaceNormal, -1.0f); // 法線を反転
						targetCol = &colB;
					}

				}
				// Sphere が存在するなら
				else {
					// Box を優先して基準にする
					if (colA.type == collision::ShapeType::Box) {
						targetCol = &colA;
					}
					else {
						surfaceNormal = Scale(surfaceNormal, -1.0f); // 法線を反転
						targetCol = &colB;
					}
				}

				// normal 方向の代表接触点を取得
				XMFLOAT3 surfaceCenter = collision::GetRepresentativeContactPointOnOBB(targetCol->worldOBB, surfaceNormal);
				// 半径取得
				float radius = GetSampleRadius(*targetCol);

				GenerateSurfaceSamplePoints(
					surfaceCenter,
					surfaceNormal,
					radius,
					r.samplePoints,
					3
				);
				constexpr float SHADOW_EPS = 0.005f;
				for (auto& p : r.samplePoints) {
					p.x += surfaceNormal.x * SHADOW_EPS;
					p.y += surfaceNormal.y * SHADOW_EPS;
					p.z += surfaceNormal.z * SHADOW_EPS;
				}


				// ShadowTestSystem にサンプルを登録
				if (shadow_collision_enabled_ && shadowTestSystem) {
					for (const auto& p : r.samplePoints) {
						shadowTestSystem->RegisterCollisionPair(eA, eB, p);
					}
				}

				// 衝突リストに追加
				contacts_.push_back({
					eA,
					eB,
					r.contact
					});
			}
		}

		// 押し出し・反発・摩擦
		for (auto& rec : contacts_) {
			const Entity eA = rec.a;
			const Entity eB = rec.b;

			// Shadow の結果を参照してスキップするか決める
			bool bothInShadow = false;
			if (shadow_collision_enabled_ && shadowTestSystem) {
				if (shadowTestSystem->AreBothInShadow(eA, eB)) {
					bothInShadow = true;
				}
			}

			// 影の中ならスキップ
			if (bothInShadow) {
				continue;
			}

			const XMFLOAT3& nAB = rec.contact.normal;

			// 押し出し量の計算
			auto [dispA, dispB] = collision::ComputePushOut(
				rec.contact,
				ecs_.GetComponent<Collider>(eA).isStatic,
				ecs_.GetComponent<Collider>(eB).isStatic,
				solve_percent_, solve_slop_);

			// 押し出し（Transform の直接更新）
			if (!ecs_.GetComponent<Collider>(eA).isStatic && !IsZeroDisp(dispA)) {
				ecs_.GetComponent<Transform>(eA).AddPosition(dispA);
			}
			if (!ecs_.GetComponent<Collider>(eB).isStatic && !IsZeroDisp(dispB)) {
				ecs_.GetComponent<Transform>(eB).AddPosition(dispB);
			}

			// 反発処理
			float invMassA = 0.0f;
			float invMassB = 0.0f;
			Rigidbody* pRbA = nullptr;
			Rigidbody* pRbB = nullptr;

			if (!ecs_.GetComponent<Collider>(eA).isStatic && ecs_.HasComponent<Rigidbody>(eA)) {
				auto& rbA = ecs_.GetComponent<Rigidbody>(eA);
				if (!rbA.isStatic && !rbA.isKinematic && rbA.mass > 0.0f) {
					invMassA = 1.0f / rbA.mass;
					pRbA = &rbA;
				}
			}
			if (!ecs_.GetComponent<Collider>(eB).isStatic && ecs_.HasComponent<Rigidbody>(eB)) {
				auto& rbB = ecs_.GetComponent<Rigidbody>(eB);
				if (!rbB.isStatic && !rbB.isKinematic && rbB.mass > 0.0f) {
					invMassB = 1.0f / rbB.mass;
					pRbB = &rbB;
				}
			}

			const float denom = invMassA + invMassB;
			if (denom <= 0.0f) { continue; }

			XMFLOAT3 vA{};
			XMFLOAT3 vB{};
			if (pRbA) { vA = pRbA->linearVelocity; }
			if (pRbB) { vB = pRbB->linearVelocity; }
			const XMFLOAT3 vRel = math::Sub(vB, vA);

			const float vRelN = math::Dot(vRel, nAB);

			float e = 0.0f;
			if (pRbA) { e = (std::max)(e, std::clamp(pRbA->restitution, 0.0f, 1.0f)); }
			if (pRbB) { e = (std::max)(e, std::clamp(pRbB->restitution, 0.0f, 1.0f)); }

			const float penetration = (std::max)(rec.contact.penetration - solve_slop_, 0.0f);
			const float bias = (penetration > 0.0f && _fixedDt > 0.0f) ? baumgarte * (penetration / _fixedDt) : 0.0f;

			float jn = -((vRelN < 0.0f ? (1.0f + e) * vRelN : vRelN) + bias) / denom;
			if (jn < 0.0f) { jn = 0.0f; }

			const XMFLOAT3 impulseN = Scale(nAB, jn);

			if (pRbA) { pRbA->linearVelocity = math::Sub(pRbA->linearVelocity, Scale(impulseN, invMassA)); }
			if (pRbB) { pRbB->linearVelocity = math::Add(pRbB->linearVelocity, Scale(impulseN, invMassB)); }

			// 摩擦
			XMFLOAT3 t = math::Sub(vRel, Scale(nAB, vRelN));
			const float tLen = math::Length(t);
			if (tLen > 1e-6f) {
				t = Scale(t, 1.0f / tLen);

				float mu = 0.0f;
				if (pRbA) { mu += std::clamp(pRbA->friction, 0.0f, 1.0f); }
				if (pRbB) { mu += std::clamp(pRbB->friction, 0.0f, 1.0f); }
				mu = (pRbA && pRbB) ? (mu * 0.5f) : mu;

				float jt = -(math::Dot(vRel, t) / denom);
				const float jtMax = mu * jn;
				jt = std::clamp(jt, -jtMax, jtMax);

				const XMFLOAT3 impulseT = Scale(t, jt);
				if (pRbA) { pRbA->linearVelocity = math::Sub(pRbA->linearVelocity, Scale(impulseT, invMassA)); }
				if (pRbB) { pRbB->linearVelocity = math::Add(pRbB->linearVelocity, Scale(impulseT, invMassB)); }
			}
		}
	}

} // namespace ecs
