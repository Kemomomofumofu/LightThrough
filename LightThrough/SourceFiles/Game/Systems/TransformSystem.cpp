
/**
 * @file TransformSystem.cpp
 * @brief TransformSystem
 * @author Arima Keita
 * @date 2025-10-03
 */

 // ---------- インクルード ---------- // 
#include <Game/Systems/TransformSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Components/Transform.h>

namespace ecs {
	/**
	 * @brief コンストラクタ
	 * @param _desc
	 */
	TransformSystem::TransformSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
	}

	/**
	 * @brief 初期化
	 */
	void TransformSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		ecs_.SetSystemSignature<TransformSystem>(signature);
	}

	/**
	 * @brief 更新処理
	 * @param _dt デルタタイム
	 */
	void TransformSystem::Update(float _dt)
	{
		using namespace DirectX;

		for (auto& e : entities_) {
			auto& tf = ecs_.GetComponent<Transform>(e);
			if (!tf.dirty) { continue; } // 変更がなければスキップ

			XMMATRIX S = XMMatrixScaling(tf.scale.x, tf.scale.y, tf.scale.z);
			XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&tf.rotationQuat));
			XMMATRIX T = XMMatrixTranslation(tf.position.x, tf.position.y, tf.position.z);
			XMMATRIX W = S * R * T;

			// ワールド行列を更新
			XMStoreFloat4x4(&tf.world, W);
			tf.dirty = false; // 更新完了
		}
	}

}


