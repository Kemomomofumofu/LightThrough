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
#include <Game/Components/Events/TriggerEvents.h>

#include <DX3D/Math/MathUtils.h>

#include <Debug/Debug.h>

namespace ecs {
	using namespace DirectX;

	namespace {


		template <class ...Ts>
		struct Overloaded : Ts... { using Ts::operator()...; };
		template <class... Ts>
		Overloaded(Ts...) -> Overloaded< Ts...>;
		// ナローフェーズの当たり判定ディスパッチャー
		std::optional<collision::ContactResult> DispatchContact(const Collider* _a, const Collider* _b)
		{
			using collision::SphereShape;
			using collision::BoxShape;

			return std::visit(
				Overloaded{
					[&](const SphereShape&, const SphereShape&) {
						return collision::IntersectSphere(_a->worldSphere, _b->worldSphere);
					},
					[&](const SphereShape&, const BoxShape&) {
						return collision::IntersectSphereOBB(_a->worldSphere, _b->worldOBB);
					},
					[&](const BoxShape&, const SphereShape&) {
						return collision::IntersectSphereOBB(_b->worldSphere, _a->worldOBB);
					},
					[&](const BoxShape&, const BoxShape&) {
						return collision::IntersectOBB(_a->worldOBB, _b->worldOBB);
					}
				},
				_a->shape, _b->shape
			);
		}

		// Transform 変更直後に Collider の world 情報を即時更新するヘルパー
		void UpdateColliderWorldFromTransform(Entity _e, Transform* _tf, Collider* _col)
		{
			switch (_col->type) {
			case collision::ShapeType::Sphere:
			{
				// スケールは最大軸で扱う（既存実装と一致させる）
				float maxScale = (std::max)({ _tf->scale.x, _tf->scale.y, _tf->scale.z });
				_col->worldSphere.center = _tf->position;
				_col->worldSphere.radius = _col->sphere.radius * maxScale;
				_col->broadPhaseRadius = _col->worldSphere.radius;
				break;
			}
			case collision::ShapeType::Box:
			{
				using namespace DirectX;
				XMVECTOR q = XMQuaternionNormalize(XMLoadFloat4(&_tf->rotationQuat));

				XMVECTOR axisX = XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), q);
				XMVECTOR axisY = XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), q);
				XMVECTOR axisZ = XMVector3Rotate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), q);

				XMStoreFloat3(&_col->worldOBB.axis[0], XMVector3Normalize(axisX));
				XMStoreFloat3(&_col->worldOBB.axis[1], XMVector3Normalize(axisY));
				XMStoreFloat3(&_col->worldOBB.axis[2], XMVector3Normalize(axisZ));

				_col->worldOBB.center = _tf->position;
				_col->worldOBB.half = {
					_col->box.halfExtents.x * _tf->scale.x,
					_col->box.halfExtents.y * _tf->scale.y,
					_col->box.halfExtents.z * _tf->scale.z
				};

				const auto& h = _col->worldOBB.half;
				_col->broadPhaseRadius = std::sqrt(h.x * h.x + h.y * h.y + h.z * h.z);
				break;
			}			default:
				break;
			}
			_col->shapeDirty = false;
		}



	} // namespace anonymous

	//! @brief コンストラクタ
	CollisionResolveSystem::CollisionResolveSystem(const SystemDesc& _desc)
		: ISystem(_desc) {
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
		// memo: 初期生成時にすり抜けてしまう問題を避けるため、最初の数フレームは影判定をスキップする (応急的な措置ではあるので、一フレームは必ずdeltaTimeを0にするような仕組みがあるといいかも。)
		constexpr float SHADOW_SKIP_Time = 1.0f;
		if (time_ < SHADOW_SKIP_Time) { time_ += _fixedDt; }
		bool skipShadowCheck = (time_ < SHADOW_SKIP_Time);

		auto shadow = shadow_test_system_.lock();

		std::unordered_set<std::pair<Entity, Entity>, EntityPairHash> currentContacts;
		// ---------- 衝突ペアの収集 ---------- //
		CollectCollisionPairs(currentContacts);

		// ---------- 影判定スキップリスト更新 ---------- //
		for (auto it = shadow_skip_pairs_.begin(); it != shadow_skip_pairs_.end();) {
			// 今回の接触リストに存在しないペアは削除
			if (currentContacts.count(*it) == 0) {
				it = shadow_skip_pairs_.erase(it);
			}
			else {
				++it;
			}
		}

		// ---------- Contact の正規化 ---------- // 
		NormalizeContacts();

		// ---------- 影判定用サンプル点登録 ---------- // 
		if (!skipShadowCheck) {
			RegisterShadowTestPoints();
		}

		// test
		shadow->ExecuteShadowTests();

		// ---------- 解決フェーズ ---------- //
		SolvePenetration();
		SolveVelocity(_fixedDt);


		// ---------- 影衝突スキップペアの更新 ---------- //
		UpdateShadowSkipPairs();

	}

	//! @brief シーン読み込み時処理
	void CollisionResolveSystem::OnSceneLoaded()
	{
		contact_records_.clear();
		shadow_skip_pairs_.clear();
		time_ = 0.0f;
	}

	//!@ 衝突ペアの収集
	void CollisionResolveSystem::CollectCollisionPairs(std::unordered_set<std::pair<Entity, Entity>, EntityPairHash>& _currentContacts)
	{
		contact_records_.clear();

		std::vector<Entity> ents(entities_.begin(), entities_.end());
		const size_t n = ents.size();

		for (size_t i = 0; i < n; ++i) {
			const Entity eA = ents[i];
			auto tfA = ecs_.GetComponent<Transform>(eA);
			auto colA = ecs_.GetComponent<Collider>(eA);
			if (colA->isTrigger) { continue; }

			for (size_t j = i + 1; j < n; ++j) {
				const Entity eB = ents[j];
				auto tfB = ecs_.GetComponent<Transform>(eB);
				auto colB = ecs_.GetComponent<Collider>(eB);
				if (colB->isTrigger) { continue; }

				const float r = colA->broadPhaseRadius + colB->broadPhaseRadius;
				if (math::DistSq(tfA->position, tfB->position) > r * r) { continue; }
				auto c = DispatchContact(colA, colB);
				if (!c || c->penetration <= 1e-6f) { continue; }

				contact_records_.push_back(ContactRecord{ eA, eB, *c, {} });
				_currentContacts.insert(std::minmax(eA, eB));
			}
		}

	}

	//! @brief 衝突法線の正規化
	void CollisionResolveSystem::NormalizeContacts()
	{
		// 以降は rec.contact.normal を唯一の法線として使う
		for (auto& rec : contact_records_) {
			auto colA = ecs_.GetComponent<Collider>(rec.a);
			auto colB = ecs_.GetComponent<Collider>(rec.b);

			// Normal を正規化
			XMFLOAT3 n = math::Normalize(rec.contact.normal);

			// 片側が static の場合、常に rec.a を dynamic 側に
			if (colA->isStatic && !colB->isStatic) {
				std::swap(rec.a, rec.b);
				n = math::Negate(n);
			}

			rec.contact.normal = n;
		}
	}

	//! @brief 影判定のサンプル点生成
	void CollisionResolveSystem::RegisterShadowTestPoints()
	{
		auto shadow = shadow_test_system_.lock();

		if (shadow_collision_enabled_ && shadow) {
			for (auto& rec : contact_records_) {
				auto baseCol = ecs_.GetComponent<Collider>(rec.a);
				auto otherCol = ecs_.GetComponent<Collider>(rec.b);
				const XMFLOAT3 n = rec.contact.normal;

				// 代表点の取得
				XMFLOAT3 center = collision::GetRepresentativeContactPointOnOBB(baseCol->worldOBB, n);
				collision::GenerateOverlapSamplePoints(baseCol->worldOBB, otherCol->worldOBB, rec.samplePoints);

				constexpr float EPS = 0.00000f; // 少しだけ法線方向にオフセットして登録
				for (auto& p : rec.samplePoints) {
					p = math::Add(p, math::Scale(n, EPS));
					shadow->RegisterCollisionPair(rec.a, rec.b, p);
				}
			}
		}

	}

	void CollisionResolveSystem::ProcessTriggerEvents()
	{
	}

	//! @brief 影判定
	void CollisionResolveSystem::ShadowFilterCollisions()
	{
		auto shadow = shadow_test_system_.lock();

		for (auto& rec : contact_records_) {
			std::pair<Entity, Entity> key = std::minmax(rec.a, rec.b);

			// 既にスキップ対象のペア
			if (shadow_skip_pairs_.count(key)) {
				rec.shadowSkiped = true;
				continue;
			}
			// 新たに両方影の中にいるペア
			if (shadow_collision_enabled_ && shadow && shadow->AreBothInShadow(rec.a, rec.b)) {
				shadow_skip_pairs_.insert(key);
				rec.shadowSkiped = true;
				continue;
			}
		}
	}

	//! @brief 重なり解決
	void CollisionResolveSystem::SolvePenetration()
	{
		for (auto& rec : contact_records_) {
			if (rec.shadowSkiped) { continue; }

			// 再取得（rec.a/rec.b は正規化後の順序）
			auto colA = ecs_.GetComponent<Collider>(rec.a);
			auto colB = ecs_.GetComponent<Collider>(rec.b);
			auto tfA = ecs_.GetComponent<Transform>(rec.a);
			auto tfB = ecs_.GetComponent<Transform>(rec.b);

			// ---------- 押し出し（位置補正） ---------- //
			auto [dispA, dispB] =
				collision::ComputePushOut(rec.contact, colA->isStatic, colB->isStatic, solve_percent_, solve_slop_);

			// Transform を直接更新（position を変えたら必ず Collider の world 情報も即時更新する）
			if (!colA->isStatic && !math::IsZeroVec(dispA)) {
				tfA->AddPosition(dispA);
				tfA->dirty = true; // Transform の dirty フラグ（Transform 実装に合わせて）
				UpdateColliderWorldFromTransform(rec.a, tfA, colA); // 即時反映
			}
			if (!colB->isStatic && !math::IsZeroVec(dispB)) {
				tfB->AddPosition(dispB);
				tfB->dirty = true;
				UpdateColliderWorldFromTransform(rec.b, tfB, colB);
			}
		}
	}

	//! @brief 加速度解決
	void CollisionResolveSystem::SolveVelocity(float _dt)
	{
		const float baumgarte = 0.2f;

		for (auto& rec : contact_records_) {
			if (rec.shadowSkiped) { continue; }

			auto colA = ecs_.GetComponent<Collider>(rec.a);
			auto colB = ecs_.GetComponent<Collider>(rec.b);

			// 両方staticはスキップ
			if (colA->isStatic && colB->isStatic) { continue; }

			// Rigidbody 必要性チェック（dynamic 側に Rigidbody が無ければ速度処理は不要）
			Rigidbody* rbA = ecs_.HasComponent<Rigidbody>(rec.a) ? ecs_.GetComponent<Rigidbody>(rec.a) : nullptr;
			Rigidbody* rbB = ecs_.HasComponent<Rigidbody>(rec.b) ? ecs_.GetComponent<Rigidbody>(rec.b) : nullptr;

			if ((!rbA || rbA->isStatic || rbA->isKinematic) && (!rbB || rbB->isStatic || rbB->isKinematic)) {
				continue;
			}

			// penetration と bias の算出
			const float pen = (std::max)(rec.contact.penetration - solve_slop_, 0.0f);
			const bool bothDynamic = !(colA->isStatic || colB->isStatic);
			const float bias = (bothDynamic && pen > 0.0f && _dt > 0.0f) ? (baumgarte * (pen / _dt)) : 0.0f;

			// 速度差（vB - vA）
			XMFLOAT3 vA{ 0,0,0 }, vB{ 0,0,0 };
			if (rbA) vA = rbA->linearVelocity;
			if (rbB) vB = rbB->linearVelocity;
			const XMFLOAT3 vRel = math::Sub(vB, vA);
			const float vRelN = math::Dot(vRel, rec.contact.normal);

			float e = 0.0f;
			if (rbA) e = (std::max)(e, std::clamp(rbA->restitution, 0.0f, 1.0f));
			if (rbB) e = (std::max)(e, std::clamp(rbB->restitution, 0.0f, 1.0f));

			// 質量係数
			const float invA = (rbA && rbA->mass > 0.0f && !rbA->isStatic && !rbA->isKinematic) ? (1.0f / rbA->mass) : 0.0f;
			const float invB = (rbB && rbB->mass > 0.0f && !rbB->isStatic && !rbB->isKinematic) ? (1.0f / rbB->mass) : 0.0f;

			const float denom = invA + invB;
			if (denom <= 1e-8f) { continue; }

			float jn = -((vRelN < 0.0f ? (1.0f + e) * vRelN : vRelN) + bias) / denom;
			if (jn < 0.0f) jn = 0.0f;

			const XMFLOAT3 impulseN = math::Scale(rec.contact.normal, jn);

			if (rbA && invA > 0.0f) {
				rbA->linearVelocity = math::Sub(rbA->linearVelocity, math::Scale(impulseN, invA));
			}
			if (rbB && invB > 0.0f) {
				rbB->linearVelocity = math::Add(rbB->linearVelocity, math::Scale(impulseN, invB));
			}

			// ---------- 摩擦処理 ---------- //
			XMFLOAT3 t = math::Sub(vRel, math::Scale(rec.contact.normal, vRelN));
			const float tLen = math::Length(t);
			if (tLen > 1e-6f) {
				t = math::Scale(t, 1.0f / tLen);

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
	}

	//! @brief スキップするペアの更新
	void CollisionResolveSystem::UpdateShadowSkipPairs()
	{
		auto shadow = shadow_test_system_.lock();

		std::unordered_set<std::pair<Entity, Entity>, EntityPairHash> newShadowSkips;
		for (const auto& rec : contact_records_) {
			if (shadow_collision_enabled_ && shadow) {
				std::pair<Entity, Entity> key = std::minmax(rec.a, rec.b);
				if (shadow_skip_pairs_.count(key)) {
					newShadowSkips.insert(key); // 衝突し続けているもののみ維持
				}
			}
		}
		shadow_skip_pairs_ = std::move(newShadowSkips);
	}

} // namespace ecs
