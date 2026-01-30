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
			if (col->isStatic && !col->shapeDirty && !tf->dirty) { continue; }

			// ワールド行列の更新
			if (tf->dirty) { tf->BuildWorld(); }

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
		float maxScale = (std::max)({ _tf->scale.x, _tf->scale.y, _tf->scale.z });
		_col->worldSphere.center = _tf->GetWorldPosition();
		_col->worldSphere.radius = s.radius * maxScale;
		_col->broadPhaseRadius = _col->worldSphere.radius;
	}


	void ColliderSyncSystem::BuildOBB(const Transform* _tf, Collider* _col)
	{
		using namespace DirectX;
		
		const auto& b = _col->box;
		// クォータニオンから直接軸を計算
		XMVECTOR q = XMQuaternionNormalize(XMLoadFloat4(&_tf->rotationQuat));
		
		// 基底軸をクォータニオンで回転
		XMVECTOR axisX = XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), q);
		XMVECTOR axisY = XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), q);
		XMVECTOR axisZ = XMVector3Rotate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), q);

		// 正規化して格納
		XMStoreFloat3(&_col->worldOBB.axis[0], XMVector3Normalize(axisX));
		XMStoreFloat3(&_col->worldOBB.axis[1], XMVector3Normalize(axisY));
		XMStoreFloat3(&_col->worldOBB.axis[2], XMVector3Normalize(axisZ));

		// 中心と半径
		_col->worldOBB.center = _tf->GetWorldPosition();
		_col->worldOBB.half = {
			b.halfExtents.x * _tf->scale.x,
			b.halfExtents.y * _tf->scale.y,
			b.halfExtents.z * _tf->scale.z,
		};

		const auto& h = _col->worldOBB.half;
		_col->broadPhaseRadius = std::sqrt(h.x * h.x + h.y * h.y + h.z * h.z);
	}
}