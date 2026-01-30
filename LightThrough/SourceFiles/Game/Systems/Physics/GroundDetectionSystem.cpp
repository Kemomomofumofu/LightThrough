/**
 * @file GroundDetectionSystem.cpp
 */

 // ---------- インクルード ---------- // 
#include <Game/Systems/Physics/GroundDetectionSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Systems/Collisions/CollisionResolveSystem.h>

#include <Game/Components/Core/Transform.h>
#include <Game/Components/Physics/GroundContact.h>
#include <Game/Components/Physics/Collider.h>

#include <Game/Collisions/CollisionUtils.h>
#include <DX3D/Math/MathUtils.h>

namespace ecs {
	//! @brief コンストラクタ
	GroundDetectionSystem::GroundDetectionSystem(const SystemDesc& desc)
		: ISystem(desc)
	{
	}

	//! @brief 初期化
	void GroundDetectionSystem::Init()
	{
		Signature sig;
		sig.set(ecs_.GetComponentType<GroundContact>());
		ecs_.SetSystemSignature<GroundDetectionSystem>(sig);

		collision_resolve_system_ = ecs_.GetSystem<CollisionResolveSystem>();
	}

	//! @brief 固定更新
	void GroundDetectionSystem::FixedUpdate(float fixedDt)
	{
		auto crs = collision_resolve_system_.lock();
		if (!crs) { return; }

		for (Entity e : entities_) {
			auto gc = ecs_.GetComponent<GroundContact>(e);
			gc->isGrounded = false;
			gc->groundNormalY = -1.0f;
		}

		constexpr float GROUND_NORMAL_Y_THRESHOLD = 0.7f;

		// 衝突結果の走査
		for (const auto& c : crs->GetContacts()) {
			const auto& n = c.contact.normal;
			DirectX::XMFLOAT3 normalForA = math::Normalize(math::Negate(n));
			DirectX::XMFLOAT3 normalForB = math::Normalize(n);

			if (normalForA.y >= GROUND_NORMAL_Y_THRESHOLD) {
				UpdateGround(c.a, normalForA);
			}
			if (normalForB.y >= GROUND_NORMAL_Y_THRESHOLD) {
				UpdateGround(c.b, normalForB);
			}
		}
	}

	/**
	 * @brief 地面接触情報更新
	 */
	void GroundDetectionSystem::UpdateGround(Entity _e, const DirectX::XMFLOAT3& _normal)
	{
		if (!ecs_.HasComponent<GroundContact>(_e)) { return; }

		auto gc = ecs_.GetComponent<GroundContact>(_e);

		// 法線を正規化
		DirectX::XMFLOAT3 n = math::Normalize(_normal);

		// 正規化結果がゼロベクトルの場合は無視
		if (std::fabs(n.x) < 1e-6f && std::fabs(n.y) < 1e-6f && std::fabs(n.z) < 1e-6f) {
			return;
		}

		// Y 成分のみを用いて最大値を採用する
		float ny = n.y;
		// gc.groundNormalY は FixedUpdate で -1.0f に初期化されている想定
		if (ny > gc->groundNormalY) {
			gc->isGrounded = true;
			gc->groundNormalY = ny;
		}
	}


} // namespace ecs
