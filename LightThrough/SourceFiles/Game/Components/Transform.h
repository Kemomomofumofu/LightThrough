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
	struct Transform {
		DirectX::XMFLOAT3 position = {};					// 位置
		DirectX::XMFLOAT4 rotationQuat = { 0.0f, 0.0f, 0.0f, 1.0f }; // 回転(クォータニオン)
		DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };	// スケール


		/**
		 * @brief モデル行列を計算する
		 * @return モデル行列
		 * [ToDo] 値が変わったら計算するようにすると軽くなる
		 */
		DirectX::XMMATRIX GetWorldMatrix() const {
			using namespace DirectX;
			XMMATRIX S = XMMatrixScaling(scale.x, scale.y, scale.z);
			XMMATRIX R = XMMatrixRotationQuaternion(XMLoadFloat4(&rotationQuat));
			XMMATRIX T = XMMatrixTranslation(position.x, position.y, position.z);

			return S * R * T;
		}
	};
}