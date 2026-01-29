/**
 * @file CollisionResolveSystem.cpp
 * @brief 押し出し・反発・摩擦を解決するシステム
 */

 // ---------- インクルード ----------
#include <optional>
#include <cmath>
#include <variant>

#include <Game/Systems/Collisions/CollisionResolveSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Systems/Gimmicks/ShadowTestSystem.h>

#include <Game/Components/Core/Transform.h>
#include <Game/Components/Physics/Collider.h>
#include <Game/Components/Physics/Rigidbody.h>

#include <DX3D/Math/MathUtils.h>

#include <Debug/Debug.h>

namespace ecs {
	using namespace DirectX;

	namespace {
		template <class ...Ts>
		struct Overloaded : Ts... { using Ts::operator()...; };
		template <class... Ts>
		Overloaded(Ts...) -> Overloaded< Ts...>;

		// ---------- 小物ユーティリティ ----------
		inline bool IsZeroVec(const XMFLOAT3& v) noexcept
		{
			return std::fabs(v.x) + std::fabs(v.y) + std::fabs(v.z) < 1e-8f;
		}
		inline XMFLOAT3 Neg(const XMFLOAT3& v) noexcept { return { -v.x, -v.y, -v.z }; }

		// Narrow phase dispatch（既存関数と整合）
		std::optional<collision::ContactResult> DispatchContact(const Collider& a, const Collider& b)
		{
			using collision::SphereShape;
			using collision::BoxShape;

			return std::visit(
				Overloaded{
					[&](const SphereShape&, const SphereShape&) {
						return collision::IntersectSphere(a.worldSphere, b.worldSphere);
					},
					[&](const SphereShape&, const BoxShape&) {
						return collision::IntersectSphereOBB(a.worldSphere, b.worldOBB);
					},
					[&](const BoxShape&, const SphereShape&) {
						return collision::IntersectSphereOBB(b.worldSphere, a.worldOBB);
					},
					[&](const BoxShape&, const BoxShape&) {
						return collision::IntersectOBB(a.worldOBB, b.worldOBB);
					}
				},
				a.shape, b.shape
			);
		}

		// サンプル点生成（影判定用）
		void GenerateSurfaceSamplePoints(
			const XMFLOAT3& center,
			const XMFLOAT3& normal,
			float radius,
			std::vector<XMFLOAT3>& out,
			int grid = 3)
		{
			XMFLOAT3 t1{}, t2{};
			if (std::fabs(normal.y) < 0.99f) {
				t1 = math::Normalize(math::Cross(normal, { 0,1,0 }));
			}
			else {
				t1 = math::Normalize(math::Cross(normal, { 1,0,0 }));
			}
			t2 = math::Normalize(math::Cross(normal, t1));

			const float step = (grid > 1) ? (2.0f * radius / (grid - 1)) : 0.0f;
			const float start = -radius;

			for (int i = 0; i < grid; ++i) {
				for (int j = 0; j < grid; ++j) {
					const float u = start + step * i;
					const float v = start + step * j;
					out.push_back(math::Add(center,
						math::Add(math::Scale(t1, u), math::Scale(t2, v))));
				}
			}
		}

		float GetSampleRadius(const Collider& col)
		{
			if (col.type == collision::ShapeType::Sphere) {
				return col.worldSphere.radius * 0.5f;
			}
			float m = (std::min)({
				col.worldOBB.half.x,
				col.worldOBB.half.y,
				col.worldOBB.half.z });
			return m * 0.8f;
		}

		// Transform 変更直後に Collider の world 情報を即時更新するヘルパー
		void UpdateColliderWorldFromTransform(Entity e, Transform& tf, Collider& col)
		{
			// Transform 側の world 行列が必要なら BuildWorld を呼ぶ（Transform 側が dirty を管理しているならそちらで）
			// ここでは Transform の position/rotation/scale の値を直接使って world 情報だけ更新する。
			switch (col.type) {
			case collision::ShapeType::Sphere:
			{
				// スケールは最大軸で扱う（既存実装と一致させる）
				float maxScale = (std::max)({ tf.scale.x, tf.scale.y, tf.scale.z });
				col.worldSphere.center = tf.position;
				col.worldSphere.radius = col.sphere.radius * maxScale;
				col.broadPhaseRadius = col.worldSphere.radius;
				break;
			}
			case collision::ShapeType::Box:
			{
				// rotationQuat から軸を計算して worldOBB.axis を更新（ColliderSync::BuildOBB と同一処理）
				using namespace DirectX;
				XMVECTOR q = XMQuaternionNormalize(XMLoadFloat4(&tf.rotationQuat));

				XMVECTOR axisX = XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), q);
				XMVECTOR axisY = XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), q);
				XMVECTOR axisZ = XMVector3Rotate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), q);

				XMStoreFloat3(&col.worldOBB.axis[0], XMVector3Normalize(axisX));
				XMStoreFloat3(&col.worldOBB.axis[1], XMVector3Normalize(axisY));
				XMStoreFloat3(&col.worldOBB.axis[2], XMVector3Normalize(axisZ));

				col.worldOBB.center = tf.position;
				col.worldOBB.half = {
					col.box.halfExtents.x * tf.scale.x,
					col.box.halfExtents.y * tf.scale.y,
					col.box.halfExtents.z * tf.scale.z
				};

				const auto& h = col.worldOBB.half;
				col.broadPhaseRadius = std::sqrt(h.x * h.x + h.y * h.y + h.z * h.z);
				break;
			}			default:
				break;
			}
			// Collider に即時反映したことを示す旗（使っているならセット）
			col.shapeDirty = false;
		}

		struct ContactRecord {
			Entity a;
			Entity b;
			collision::ContactResult contact;
			std::vector<XMFLOAT3> samplePoints;
		};

	} // namespace anonymous

	// ---------- System 本体 ----------

	CollisionResolveSystem::CollisionResolveSystem(const SystemDesc& desc)
		: ISystem(desc) {
	}

	void CollisionResolveSystem::Init()
	{
		Signature sig;
		sig.set(ecs_.GetComponentType<Transform>());
		sig.set(ecs_.GetComponentType<Collider>());
		ecs_.SetSystemSignature<CollisionResolveSystem>(sig);

		shadow_test_system_ = ecs_.GetSystem<ShadowTestSystem>();
	}

	void CollisionResolveSystem::FixedUpdate(float fixedDt)
	{
		auto shadow = shadow_test_system_.lock();
		contacts_.clear();

		// ---------- 接触収集 ----------
		std::vector<Entity> ents(entities_.begin(), entities_.end());
		const size_t n = ents.size();

		for (size_t i = 0; i < n; ++i) {
			const Entity eA = ents[i];
			auto& tfA = ecs_.GetComponent<Transform>(eA);
			auto& colA = ecs_.GetComponent<Collider>(eA);
			if (colA.isTrigger) { continue; }

			for (size_t j = i + 1; j < n; ++j) {
				const Entity eB = ents[j];
				auto& tfB = ecs_.GetComponent<Transform>(eB);
				auto& colB = ecs_.GetComponent<Collider>(eB);
				if (colB.isTrigger) { continue; }

				const float r = colA.broadPhaseRadius + colB.broadPhaseRadius;
				if (math::DistSq(tfA.position, tfB.position) > r * r) { continue; }

				auto c = DispatchContact(colA, colB);
				if (!c || c->penetration <= 1e-6f) { continue; }

				contacts_.push_back(ContactRecord{ eA, eB, *c, {} });
			}
		}

		// ---------- Contact の正規化 ----------
		// ここで normal の向きを一度だけ確定する（以降は rec.contact.normal を唯一の法線として使う）
		for (auto& rec : contacts_) {
			auto& colA = ecs_.GetComponent<Collider>(rec.a);
			auto& colB = ecs_.GetComponent<Collider>(rec.b);

			// Normal を正規化
			XMFLOAT3 n = math::Normalize(rec.contact.normal);

			// 片側が static の場合、常に rec.a を dynamic 側にする（取り扱い単純化）
			if (colA.isStatic && !colB.isStatic) {
				std::swap(rec.a, rec.b);
				n = Neg(n);
			}
			// ここで rec.a が常に dynamic 側（少なくとも one-of が dynamic の時）になっていることに注意
			rec.contact.normal = n;
		}

		// ---------- Shadow サンプル（確定 normal を使う） ----------
		if (shadow_collision_enabled_ && shadow) {
			for (auto& rec : contacts_) {
				auto& baseCol = ecs_.GetComponent<Collider>(rec.a); // rec.a は base（サンプリング基準）
				const XMFLOAT3 n = rec.contact.normal;

				// 代表点の取得は ColliderUtils の関数を利用
				XMFLOAT3 center = collision::GetRepresentativeContactPointOnOBB(baseCol.worldOBB, n);
				float radius = GetSampleRadius(baseCol);
				GenerateSurfaceSamplePoints(center, n, radius, rec.samplePoints);

				constexpr float EPS = 0.005f;
				for (auto& p : rec.samplePoints) {
					p = math::Add(p, math::Scale(n, EPS));
					shadow->RegisterCollisionPair(rec.a, rec.b, p);
				}
			}
		}

		// ---------- 解決フェーズ ----------
		const float baumgarte = 0.2f;

		for (auto& rec : contacts_) {

			// Shadow によって衝突自体を無視するケース
			if (shadow_collision_enabled_ && shadow) {
				if (shadow->AreBothInShadow(rec.a, rec.b)) { continue; }
			}

			// 再取得（rec.a/rec.b は正規化後の順序）
			auto& colA = ecs_.GetComponent<Collider>(rec.a);
			auto& colB = ecs_.GetComponent<Collider>(rec.b);
			auto& tfA = ecs_.GetComponent<Transform>(rec.a);
			auto& tfB = ecs_.GetComponent<Transform>(rec.b);

			// ---------- 押し出し（位置補正） ----------
			auto [dispA, dispB] =
				collision::ComputePushOut(rec.contact, colA.isStatic, colB.isStatic, solve_percent_, solve_slop_);

			// Transform を直接更新（position を変えたら必ず Collider の world 情報も即時更新する）
			if (!colA.isStatic && !IsZeroVec(dispA)) {
				tfA.AddPosition(dispA);
				tfA.dirty = true; // Transform の dirty フラグ（Transform 実装に合わせて）
				UpdateColliderWorldFromTransform(rec.a, tfA, colA); // 即時反映
			}
			if (!colB.isStatic && !IsZeroVec(dispB)) {
				tfB.AddPosition(dispB);
				tfB.dirty = true;
				UpdateColliderWorldFromTransform(rec.b, tfB, colB);
			}

			// ---------- インパルス解決 ----------
			// ここでは "Static-Static" はスキップ、"Dynamic-Dynamic" と "Static-Dynamic" を扱う
			if (colA.isStatic && colB.isStatic) {
				continue; // 両方 static => 何もしない
			}

			// Rigidbody 必要性チェック（dynamic 側に Rigidbody が無ければ速度処理は不要）
			Rigidbody* rbA = ecs_.HasComponent<Rigidbody>(rec.a) ? &ecs_.GetComponent<Rigidbody>(rec.a) : nullptr;
			Rigidbody* rbB = ecs_.HasComponent<Rigidbody>(rec.b) ? &ecs_.GetComponent<Rigidbody>(rec.b) : nullptr;

			// 動的側がいずれか存在しない場合はスキップ
			if ((!rbA || rbA->isStatic || rbA->isKinematic) && (!rbB || rbB->isStatic || rbB->isKinematic)) {
				continue;
			}

			// penetration と bias の算出
			const float pen = (std::max)(rec.contact.penetration - solve_slop_, 0.0f);

			// 判定: 双方 dynamic か、それ以外（片側 static）
			const bool bothDynamic = !(colA.isStatic || colB.isStatic);

			// bias は基本的に dynamic-dynamic のみ効かせ、static-dynamic では 0 にして静的オブジェクトが「速度によって」押される事を防ぐ
			const float bias = (bothDynamic && pen > 0.0f && fixedDt > 0.0f) ? (baumgarte * (pen / fixedDt)) : 0.0f;

			// 速度差（vB - vA）
			XMFLOAT3 vA{ 0,0,0 }, vB{ 0,0,0 };
			if (rbA) vA = rbA->linearVelocity;
			if (rbB) vB = rbB->linearVelocity;
			const XMFLOAT3 vRel = math::Sub(vB, vA);
			const float vRelN = math::Dot(vRel, rec.contact.normal);

			float e = 0.0f;
			if (rbA) e = (std::max)(e, std::clamp(rbA->restitution, 0.0f, 1.0f));
			if (rbB) e = (std::max)(e, std::clamp(rbB->restitution, 0.0f, 1.0f));

			// 質量係数（片側 static の場合はその側は invMass = 0）
			const float invA = (rbA && rbA->mass > 0.0f && !rbA->isStatic && !rbA->isKinematic) ? (1.0f / rbA->mass) : 0.0f;
			const float invB = (rbB && rbB->mass > 0.0f && !rbB->isStatic && !rbB->isKinematic) ? (1.0f / rbB->mass) : 0.0f;

			const float denom = invA + invB;
			if (denom <= 1e-8f) { continue; }

			float jn = -((vRelN < 0.0f ? (1.0f + e) * vRelN : vRelN) + bias) / denom;
			if (jn < 0.0f) jn = 0.0f;

			const XMFLOAT3 impulseN = math::Scale(rec.contact.normal, jn);

			// Static-動的 の場合でも、静的側には速度を与えない（invMass==0 のため実際は影響しないが明確にする）
			if (rbA && invA > 0.0f) {
				rbA->linearVelocity = math::Sub(rbA->linearVelocity, math::Scale(impulseN, invA));
			}
			if (rbB && invB > 0.0f) {
				rbB->linearVelocity = math::Add(rbB->linearVelocity, math::Scale(impulseN, invB));
			}

			// ---------- 摩擦処理 ----------
			XMFLOAT3 t = math::Sub(vRel, math::Scale(rec.contact.normal, vRelN));
			const float tLen = math::Length(t);
			if (tLen > 1e-6f) {
				t = math::Scale(t, 1.0f / tLen);

				// 摩擦係数: 双方が持つ場合は平均、それ以外は持っている方の値を使う
				float muA = (rbA) ? std::clamp(rbA->friction, 0.0f, 1.0f) : 0.0f;
				float muB = (rbB) ? std::clamp(rbB->friction, 0.0f, 1.0f) : 0.0f;
				float mu = (rbA && rbB) ? ((muA + muB) * 0.5f) : (muA + muB);

				float jt = -(math::Dot(vRel, t) / denom);
				const float jtMax = mu * jn;
				jt = std::clamp(jt, -jtMax, jtMax);

				const XMFLOAT3 impulseT = math::Scale(t, jt);
				if (rbA && invA > 0.0f) {
					rbA->linearVelocity = math::Sub(rbA->linearVelocity, math::Scale(impulseT, invA));
				}
				if (rbB && invB > 0.0f) {
					rbB->linearVelocity = math::Add(rbB->linearVelocity, math::Scale(impulseT, invB));
				}
			}
		}

		// end FixedUpdate
	}

} // namespace ecs
