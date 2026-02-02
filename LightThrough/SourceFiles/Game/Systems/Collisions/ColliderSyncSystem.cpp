/**
 * @file ColliderSyncSystem.cpp
 * @brief コライダー同期システム
  * @author Arima Keita
 * @date 2025-10-03
 */


 // ---------- インクルード ---------- // 
#include <DirectXMath.h>
#include <Game/Systems/Collisions/ColliderSyncSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Core/Transform.h>
#include <Game/Components/Physics/Collider.h>

#include <DX3D/Math/MathUtils.h>



namespace ecs {
	ColliderSyncSystem::ColliderSyncSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{

	}

	void ColliderSyncSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<Collider>());
		ecs_.SetSystemSignature<ColliderSyncSystem>(signature);
	}


	/**
	 * @brief 形状の構築
	 * @param _dt
	 */
	void ColliderSyncSystem::FixedUpdate(float _fixedDt)
	{
		for (auto e : entities_) {
			auto tf = ecs_.GetComponent<Transform>(e);
			auto col = ecs_.GetComponent<Collider>(e);

			// Transformに変更がなかった場合のみ更新
			//if (!col->shapeDirty && !tf->dirty) { continue; }

			// 形状の更新
			switch (col->type) {
			case collision::ShapeType::Sphere:
			{
				BuildSphere(tf, col);
				break;
			}
			case collision::ShapeType::Box:
			{
				BuildOBB(tf, col);
				break;
			}
			default:
				break;
			}

			// 変更フラグをリセット
			col->shapeDirty = false;
		}
	}

	void ColliderSyncSystem::BuildSphere(const Transform* _tf, Collider* _col)
	{
		const auto& s = _col->sphere;
		const auto ws = _tf->GetWorldScaleCached();
		float maxScale = (std::max)({ ws.x, ws.y, ws.z });
		_col->worldSphere.center = _tf->GetWorldPosition();
		_col->worldSphere.radius = s.radius * maxScale;
		_col->broadPhaseRadius = _col->worldSphere.radius;
	}


	void ColliderSyncSystem::BuildOBB(const Transform* _tf, Collider* _col)
	{
		using namespace DirectX;

		const auto& b = _col->box;

		// 軸の取得
		XMFLOAT3 r = _tf->GetWorldRightCached();
		XMFLOAT3 u = _tf->GetWorldUpCached();
		XMFLOAT3 f = _tf->GetWorldForwardCached();


		_col->worldOBB.axis[0] = math::Normalize(r);
		_col->worldOBB.axis[1] = math::Normalize(u);
		_col->worldOBB.axis[2] = math::Normalize(f);

		// 中心と半径
		_col->worldOBB.center = _tf->GetWorldPosition();

		const auto ws = _tf->GetWorldScaleCached();
		_col->worldOBB.half = {
			b.halfExtents.x * ws.x,
			b.halfExtents.y * ws.y,
			b.halfExtents.z * ws.z,
		};
		const auto& h = _col->worldOBB.half;
		_col->broadPhaseRadius = math::Length(h);
	}
}