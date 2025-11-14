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
		XMFLOAT3 position = { 0, 0, 0 };					// 位置
		XMFLOAT4 rotationQuat = { 0, 0, 0, 1 }; // 回転(クォータニオン)
		XMFLOAT3 scale = { 1, 1, 1 };	// スケール

		// キャッシュ用
		mutable XMFLOAT4X4 world{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
		};

		mutable bool dirty = true; // 変更フラグ

		mutable XMFLOAT3 forward{ 0.0f, 0.0f, 1.0f }; //! 前方向ベクトル(キャッシュ用)
		mutable XMFLOAT3 up{ 0.0f, 1.0f, 0.0f };      //! 上方向ベクトル(キャッシュ用)
		mutable XMFLOAT3 right{ 1.0f, 0.0f, 0.0f };   //! 右方向ベクトル(キャッシュ用)
		mutable bool axesDirty = true;          //! 方向ベクトル更新フラグ

		// セッターを介して変更することで自動的にDirtyフラグを立てる
		/**
		 * @brief 位置更新
		 * @param _pos 更新後の位置
		 */
		void SetPosition(const XMFLOAT3& _pos) {
			position = _pos;
			dirty = true;
		}
		void AddPosition(const XMFLOAT3& _pos) {
			position.x += _pos.x;
			position.y += _pos.y;
			position.z += _pos.z;
			dirty = true;
		}

		/**
		 * @brief 回転更新
		 * @param _quat 更新後の回転(クォータニオン)
		 */
		void SetRotation(const XMFLOAT4& _quat) {
			rotationQuat = _quat;
			dirty = true;
			axesDirty = true;
		}
		void AddRotation(const XMFLOAT4& _quat) {
			XMVECTOR r1 = XMLoadFloat4(&rotationQuat);
			XMVECTOR r2 = XMLoadFloat4(&_quat);
			XMVECTOR r = XMQuaternionMultiply(r2, r1);
			XMStoreFloat4(&rotationQuat, r);
			dirty = true;
			axesDirty = true;
		}

		/**
		 * @brief スケール更新
		 * @param _scale 更新後のスケール
		 */
		void SetScale(const XMFLOAT3& _scale) {
			scale = _scale;
			dirty = true;
		}
		void AddScale(const XMFLOAT3& _scale) {
			scale.x += _scale.x;
			scale.y += _scale.y;
			scale.z += _scale.z;
			dirty = true;
		}

		//! @brief ワールド行列の更新
		void BuildWorld()
		{
			if (!dirty) { return; }

			XMVECTOR s = XMLoadFloat3(&scale);
			XMVECTOR r = XMLoadFloat4(&rotationQuat);
			XMVECTOR t = XMLoadFloat3(&position);
			XMMATRIX M = XMMatrixAffineTransformation(s, XMVectorZero(), r, t);

			XMStoreFloat4x4(&world, M);
			dirty = false;
		}

		//! @brief 方向の更新
		void RecalcAxes() const
		{
			if (!axesDirty) { return; }

			XMVECTOR q = XMQuaternionNormalize(XMLoadFloat4(&rotationQuat));
			auto rotN = [&](float _x, float _y, float _z) {
				return XMVector3Normalize(XMVector3Rotate(XMVectorSet(_x, _y, _z, 0), q));
				};

			XMStoreFloat3(&forward, rotN(0, 0, 1));
			XMStoreFloat3(&right, rotN(1, 0, 0));
			XMStoreFloat3(&up, rotN(0, 1, 0));
			axesDirty = false;

		}

		//! @brief 前方向取得
		const XMFLOAT3& GetForward() const
		{
			RecalcAxes();
			return forward;
		}
		//! @brief 右方向取得
		const XMFLOAT3& GetRight() const
		{
			RecalcAxes();
			return right;
		}
		//! @brief 上方向取得
		const XMFLOAT3& GetUp() const
		{
			RecalcAxes();
			return up;
		}

	};
}


ECS_REFLECT_BEGIN(ecs::Transform)
ECS_REFLECT_FIELD(position),
ECS_REFLECT_FIELD(rotationQuat),
ECS_REFLECT_FIELD(scale)
ECS_REFLECT_END()
