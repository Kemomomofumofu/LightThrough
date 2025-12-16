/**
 * @file IntegrationSystem.cpp
 * @brief 速度をtransformに反映する
 */


 // ---------- インクルード ---------- // 
#include <Game/Systems/Physics/IntegrationSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Physics/Rigidbody.h>

namespace ecs {
	void IntegrationSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<ecs::Transform>());
		signature.set(ecs_.GetComponentType<ecs::Rigidbody>());
		ecs_.SetSystemSignature<IntegrationSystem>(signature);
	}

	void IntegrationSystem::FixedUpdate(float _fixedDt)
	{
		using namespace DirectX;

		for (auto& e : entities_) {
			auto& tf = ecs_.GetComponent<ecs::Transform>(e);
			auto& rb = ecs_.GetComponent<ecs::Rigidbody>(e);

			if (rb.isStatic || rb.isKinematic) { continue; }

			// 質量の逆数 todo: 初期化時にキャッシュしといてもいいかもしれない。
			const float invMass = (rb.mass > 0.0f) ? (1.0f / rb.mass) : 0.0f;

			// 線形加速度: (F / m)
			const XMFLOAT3 accel{
				rb.force.x * invMass,
				rb.force.y * invMass,
				rb.force.z * invMass
			};
			// memo: Semi-Implicit Euler法
			rb.linearVelocity.x += accel.x * _fixedDt;
			rb.linearVelocity.y += accel.y * _fixedDt;
			rb.linearVelocity.z += accel.z * _fixedDt;

			// 位置更新
			XMFLOAT3 add = {
				rb.linearVelocity.x * _fixedDt,
				rb.linearVelocity.y * _fixedDt,
				rb.linearVelocity.z * _fixedDt
			};
			tf.AddPosition(add);

			// 角運動 memo: 慣性テンソル未実装なので、現状は角加速度は未計算
			// 各速度ベクトルから微小回転Δqを作り、 q <- Δq * q を計算して回転を更新する。
			{
				XMVECTOR w = XMLoadFloat3(&rb.angularVelocity);
				float wLen = XMVectorGetX(XMVector3Length(w));
				if (wLen > 0.0f) {
					const float angle = wLen * _fixedDt;
					XMVECTOR axis = XMVectorScale(w, 1.0f / wLen);

					XMVECTOR q = XMLoadFloat4(&tf.rotationQuat);
					XMVECTOR dq = XMQuaternionRotationAxis(axis, angle);
					q = XMQuaternionNormalize(XMQuaternionMultiply(dq, q));
					XMStoreFloat4(&tf.rotationQuat, q);
				}
			}

			tf.dirty = true;
		}
	}

}
