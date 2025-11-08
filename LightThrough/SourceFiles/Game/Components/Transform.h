#pragma once
/**
 * @file Transform.h
 * @brief 姿勢制御コンポーネント
 * @author Arima Keita
 * @date 2025-08-08
 */

// ---------- インクルード ---------- // 
#include <DirectXMath.h>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {
	using namespace DirectX;

	struct Transform {
		XMFLOAT3 position = {};					// 位置
		XMFLOAT4 rotationQuat = { 0.0f, 0.0f, 0.0f, 1.0f }; // 回転(クォータニオン)
		XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };	// スケール

		// キャッシュ用
		XMFLOAT4X4 world{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};

		bool dirty = true; // 変更フラグ
		

		// セッターを介して変更することで自動的にDirtyフラグを立てる
		/**
		 * @brief 位置更新
		 * @param _pos 更新後の位置
		 */
		inline void SetPosition(const XMFLOAT3& _pos) {
			position = _pos;
			dirty = true;
		}
		inline void AddPosition(const XMFLOAT3& _pos) {
			position.x += _pos.x;
			position.y += _pos.y;
			position.z += _pos.z;
			dirty = true;
		}

		/**
		 * @brief 回転更新
		 * @param _quat 更新後の回転(クォータニオン)
		 */
		inline void SetRotation(const XMFLOAT4& _quat) {
			rotationQuat = _quat;
			dirty = true;
		}
		inline void AddRotation(const XMFLOAT4& _quat) {
			XMVECTOR r1 = XMLoadFloat4(&rotationQuat);
			XMVECTOR r2 = XMLoadFloat4(&_quat);
			XMVECTOR r = XMQuaternionMultiply(r2, r1);
			XMStoreFloat4(&rotationQuat, r);
			dirty = true;
		}

		/**
		 * @brief スケール更新
		 * @param _scale 更新後のスケール
		 */
		inline void SetScale(const XMFLOAT3& _scale) {
			scale = _scale;
			dirty = true;
		}
		inline void AddScale(const XMFLOAT3& _scale) {
			scale.x += _scale.x;
			scale.y += _scale.y;
			scale.z += _scale.z;
			dirty = true;
		}

		/**
		 * @brief ワールド行列の更新
		 */
		inline void BuildWorld() {
			if (!dirty) { return; }

			XMVECTOR s = XMLoadFloat3(&scale);
			XMVECTOR r = XMLoadFloat4(&rotationQuat);
			XMVECTOR t = XMLoadFloat3(&position);
			XMMATRIX M = XMMatrixAffineTransformation(s, XMVectorZero(), r, t);

			XMStoreFloat4x4(&world, M);
			dirty = false;
		}
	};
}


ECS_REFLECT_BEGIN(ecs::Transform)
ECS_REFLECT_FIELD(position),
ECS_REFLECT_FIELD(rotationQuat),
ECS_REFLECT_FIELD(scale)
ECS_REFLECT_END()
