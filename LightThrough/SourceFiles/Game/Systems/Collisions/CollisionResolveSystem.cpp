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

#include <Game/Components/Core/Transform.h>
#include <Game/Components/Physics/Collider.h>
#include <Game/Components/Physics/Rigidbody.h>
#include <Game/Components/Physics/GroundContact.h>
#include <Game/Components/Core/Name.h>

#include <Game/Collisions/CollisionUtils.h>
#include <Debug/Debug.h>

namespace ecs {
	using namespace DirectX;

	namespace {
		constexpr float GROUND_NORMAL_Y_THRESHOLD = -0.7f; // 地面とみなす法線のY成分閾値

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

		/**
		 * @brief 法線方向に面しているOBBの4頂点を取得
		 * @param _obb OBB
		 * @param _normal 法線方向（この方向を向いている面の頂点を取得）
		 * @param _outPoints 出力先（4点）
		 */
		void GetOBBFacePointsAlongNormal(const collision::WorldOBB& _obb, const XMFLOAT3& _normal, std::vector<XMFLOAT3>& _outPoints)
		{
			// 8頂点を取得
			XMFLOAT3 corners[8];
			collision::GetOBBCorners(_obb, corners);

			// 法線方向に最も近い4点を選択
			// 法線とOBB中心からの方向を比較して、法線側の4点を取得
			XMVECTOR nVec = XMLoadFloat3(&_normal);
			XMVECTOR center = XMLoadFloat3(&_obb.center);

			// 各頂点と中心の差分ベクトルと法線の内積を計算
			std::vector<std::pair<float, int>> dotProducts;
			for (int i = 0; i < 8; ++i) {
				XMVECTOR corner = XMLoadFloat3(&corners[i]);
				XMVECTOR toCorner = XMVectorSubtract(corner, center);
				float dot = XMVectorGetX(XMVector3Dot(toCorner, nVec));
				dotProducts.push_back({ dot, i });
			}

			// 内積が大きい順（法線方向に近い順）にソート
			std::sort(dotProducts.begin(), dotProducts.end(),
				[](const auto& a, const auto& b) { return a.first > b.first; });

			// 上位4点を追加
			for (int i = 0; i < 4; ++i) {
				_outPoints.push_back(corners[dotProducts[i].second]);
			}
		}

		/**
		 * @brief 法線方向に面しているSphereのサンプル点を取得
		 * @param _sphere Sphere
		 * @param _normal 法線方向
		 * @param _outPoints 出力先
		 */
		void GetSphereFacePointsAlongNormal(const collision::WorldSphere& _sphere, const XMFLOAT3& _normal, std::vector<XMFLOAT3>& _outPoints)
		{
			// 法線方向の1点 + 周囲4点（法線に垂直な平面上）
			XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&_normal));
			XMVECTOR center = XMLoadFloat3(&_sphere.center);

			// 法線方向の点
			XMFLOAT3 mainPoint;
			XMStoreFloat3(&mainPoint, XMVectorAdd(center, XMVectorScale(n, _sphere.radius)));
			_outPoints.push_back(mainPoint);

			// 法線に垂直な2つの軸を計算
			XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			if (std::fabs(XMVectorGetX(XMVector3Dot(n, up))) > 0.9f) {
				up = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
			}
			XMVECTOR tangent1 = XMVector3Normalize(XMVector3Cross(n, up));
			XMVECTOR tangent2 = XMVector3Cross(n, tangent1);

			// 周囲4点（少し法線方向にオフセット）
			float offsetAlongNormal = _sphere.radius * 0.7f;  // 法線方向に70%
			float offsetPerpendicular = _sphere.radius * 0.7f; // 垂直方向に70%

			XMVECTOR basePos = XMVectorAdd(center, XMVectorScale(n, offsetAlongNormal));

			XMFLOAT3 p;
			XMStoreFloat3(&p, XMVectorAdd(basePos, XMVectorScale(tangent1, offsetPerpendicular)));
			_outPoints.push_back(p);
			XMStoreFloat3(&p, XMVectorAdd(basePos, XMVectorScale(tangent1, -offsetPerpendicular)));
			_outPoints.push_back(p);
			XMStoreFloat3(&p, XMVectorAdd(basePos, XMVectorScale(tangent2, offsetPerpendicular)));
			_outPoints.push_back(p);
			XMStoreFloat3(&p, XMVectorAdd(basePos, XMVectorScale(tangent2, -offsetPerpendicular)));
			_outPoints.push_back(p);
		}

	} // unnamed namespace


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

		// GroundContactの初期化
		for (Entity e : entities_) {
			if (ecs_.HasComponent<GroundContact>(e)) {
				auto& gc = ecs_.GetComponent<GroundContact>(e);
				gc.isGrounded = false;
				gc.groundNormalY = 0.0f;
			}
		}


		// ペアごとに処理するためにベクターにコピー
		std::vector<Entity> ents;
		ents.reserve(entities_.size());
		for (const auto& e : entities_) { ents.push_back(e); };

		const size_t n = ents.size();
		const float baumgarte = 0.2f;

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
				if (DistSq(tfA.position, tfB.position) > sumR * sumR) { continue; }

				// ナローフェーズ
				const auto contact = DispatchContact(colA, colB);
				if (!contact) { continue; }
				if (contact->penetration <= 1e-6f) { continue; }

				// 影判定用の接触点を登録
				if (shadow_collision_enabled_ && shadowTestSystem) {
					const XMFLOAT3 normalToB = contact->normal;
					const XMFLOAT3 normalToA = Scale(normalToB, -1.0f);

					std::vector<XMFLOAT3> contactPoints;
					contactPoints.reserve(10);

					// オフセット量（潜り込み分 + 余裕）
					constexpr float SURFACE_OFFSET = 0.05f;

					// Aの接触面の点を取得（Bに向かう面）→ 法線方向にオフセット
					if (colA.type == collision::ShapeType::Box) {
						GetOBBFacePointsAlongNormal(colA.worldOBB, normalToB, contactPoints);
					}
					else if (colA.type == collision::ShapeType::Sphere) {
						GetSphereFacePointsAlongNormal(colA.worldSphere, normalToB, contactPoints);
					}

					// Aの点を法線の逆方向（外側）にオフセット
					size_t aPointCount = contactPoints.size();
					for (size_t i = 0; i < aPointCount; ++i) {
						// Aの表面から外側へ（normalToAの方向 = Bから離れる方向）
						contactPoints[i].x += normalToA.x * SURFACE_OFFSET;
						contactPoints[i].y += normalToA.y * SURFACE_OFFSET;
						contactPoints[i].z += normalToA.z * SURFACE_OFFSET;
					}

					// Bの接触面の点を取得（Aに向かう面）
					size_t bPointStart = contactPoints.size();
					if (colB.type == collision::ShapeType::Box) {
						GetOBBFacePointsAlongNormal(colB.worldOBB, normalToA, contactPoints);
					}
					else if (colB.type == collision::ShapeType::Sphere) {
						GetSphereFacePointsAlongNormal(colB.worldSphere, normalToA, contactPoints);
					}

					// Bの点を法線の逆方向（外側）にオフセット
					for (size_t i = bPointStart; i < contactPoints.size(); ++i) {
						// Bの表面から外側へ（normalToBの方向 = Aから離れる方向）
						contactPoints[i].x += normalToB.x * SURFACE_OFFSET;
						contactPoints[i].y += normalToB.y * SURFACE_OFFSET;
						contactPoints[i].z += normalToB.z * SURFACE_OFFSET;
					}

					DebugLogInfo("[CollisionResolve] Registering {} contact points for entities {} and {}",
						contactPoints.size(), eA.id_, eB.id_);

					for (const auto& point : contactPoints) {
						shadowTestSystem->RegisterCollisionPair(eA, eB, point);
					}

					// 前フレームの結果を確認（すり抜け判定）
					bool inShadow = shadowTestSystem->AreBothInShadow(eA, eB);
					DebugLogInfo("[CollisionResolve] AreBothInShadow({}, {}) = {}", eA.id_, eB.id_, inShadow);
					
					if (inShadow) { continue; }
				}

				// 設置判定の更新
				auto tryUpdateGroundContact = [&](Entity _self, Entity _other, const XMFLOAT3& _normal)
					{
						if (!ecs_.HasComponent<GroundContact>(_self)) { return; }
						if (_normal.y > GROUND_NORMAL_Y_THRESHOLD) { return; }

						auto& gc = ecs_.GetComponent<GroundContact>(_self);
						gc.isGrounded = true;

						if (_normal.y > gc.groundNormalY) {
							gc.groundNormalY = _normal.y;
						}
					};
				const XMFLOAT3& nAB = contact->normal;
				tryUpdateGroundContact(eA, eB, nAB);
				tryUpdateGroundContact(eB, eA, Scale(nAB, -1.0f));


				// 押し出し量の計算 (A->B 法線)
				auto [dispA, dispB] = collision::ComputePushOut(
					*contact,
					colA.isStatic, colB.isStatic,
					solve_percent_, solve_slop_);

				// 押し出し
				if (!colA.isStatic && !IsZeroDisp(dispA)) {
					tfA.AddPosition(dispA);
				}
				if (!colB.isStatic && !IsZeroDisp(dispB)) {
					tfB.AddPosition(dispB);
				}

				// 反発処理（既存コードと同じ）
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


		if (shadow_collision_enabled_ && shadowTestSystem) {
			shadowTestSystem->ExecuteShadowTests();
		}
	}
} // namespace ecs
