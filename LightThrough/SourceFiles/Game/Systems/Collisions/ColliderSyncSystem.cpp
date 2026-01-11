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

#include <Game/Components/Transform.h>
#include <Game/Components/Collider.h>



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
	 * @brief 球形状の構築
	 * @param _dt
	 */
	void ColliderSyncSystem::FixedUpdate(float _fixedDt)
	{
		for (auto e : entities_) {
			auto& tf = ecs_.GetComponent<Transform>(e);
			auto& col = ecs_.GetComponent<Collider>(e);

			// Transformに変更があった場合のみ更新
			if (col.isStatic && !col.shapeDirty && !tf.dirty) { continue; }

			// ワールド行列の更新
			if (tf.dirty) { tf.BuildWorld(); }

			// 形状の更新
			switch (col.type) {
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
			col.shapeDirty = false;
			tf.dirty = false;
		}
	}

	void ColliderSyncSystem::BuildSphere(const Transform& _tf, Collider& _col)
	{
		const auto& s = _col.sphere;
		float maxScale = (std::max)({ _tf.scale.x, _tf.scale.y, _tf.scale.z });
		_col.worldSphere.center = _tf.position;
		_col.worldSphere.radius = s.radius * maxScale;
		_col.broadPhaseRadius = _col.worldSphere.radius;
	}


	void ColliderSyncSystem::BuildOBB(const Transform& _tf, Collider& _col)
	{
		const auto& b = _col.box;

		// 行列軸の取得
		XMFLOAT3 rawX{ _tf.world._11, _tf.world._12, _tf.world._13 };
		XMFLOAT3 rawY{ _tf.world._21, _tf.world._22, _tf.world._23 };
		XMFLOAT3 rawZ{ _tf.world._31, _tf.world._32, _tf.world._33 };

		// 各軸の長さ
		auto len = [](const XMFLOAT3& _v) {
			return std::sqrt(_v.x * _v.x + _v.y * _v.y + _v.z * _v.z);
			};
		float lx = len(rawX);
		float ly = len(rawY);
		float lz = len(rawZ);

		// 正規化
		auto norm = [&](const XMFLOAT3& _v, float _l) -> XMFLOAT3 {
			if (_l < 1e-8f) { return XMFLOAT3{ 0, 0, 0 }; }
			return { _v.x / _l, _v.y / _l, _v.z / _l };
			};

		_col.worldOBB.center = _tf.position;
		_col.worldOBB.axis[0] = norm(rawX, lx);
		_col.worldOBB.axis[1] = norm(rawY, ly);
		_col.worldOBB.axis[2] = norm(rawZ, lz);
		_col.worldOBB.half = {
			b.halfExtents.x * lx,
			b.halfExtents.y * ly,
			b.halfExtents.z * lz,
		};

		const auto& h = _col.worldOBB.half;
		_col.broadPhaseRadius = std::sqrt(h.x * h.x + h.y * h.y + h.z * h.z);
	}
}