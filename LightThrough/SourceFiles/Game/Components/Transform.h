#pragma once
/**
 * @file Transform.h
 * @brief 姿勢制御コンポーネント
 * @author Arima Keita
 * @date 2025-08-08
 */

// ---------- インクルード ---------- // 
#include <DirectXMath.h>

namespace ecs {
	using namespace DirectX;

	struct Transform {
		XMFLOAT3 position = {};					// 位置
		XMFLOAT4 rotationQuat = { 0.0f, 0.0f, 0.0f, 1.0f }; // 回転(クォータニオン)
		XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };	// スケール

		// キャッシュ用
		mutable XMFLOAT4X4 world{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};

		mutable bool dirty = true; // 変更フラグ
		

		// セッターを介して変更することで自動的にdirtyフラグを立てる
		/**
		 * @brief 位置更新
		 * @param _pos 更新後の位置
		 */
		inline void SetPosition(const XMFLOAT3& _pos) {
			position = _pos;
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

		/**
		 * @brief スケール更新
		 * @param _scale 更新後のスケール
		 */
		inline void SetScale(const XMFLOAT3& _scale) {
			scale = _scale;
			dirty = true;
		}

		/**
		 * @brief ワールド行列の更新
		 */
		inline void BuildWorld() const {
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
