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
		XMFLOAT3 position = { 0, 0, 0 };					//! 位置
		XMFLOAT4 rotationQuat = { 0, 0, 0, 1 };				//! 回転(クォータニオン)
		XMFLOAT3 scale = { 1, 1, 1 };						//! スケール

		//! @brief エディター表示用の角度キャッシュ
		//! @memo ImGuiで編集する用の変数、実際の値はrotationQuatで扱う
		mutable XMFLOAT3 eulerDegCache{ 0.0f, 0.0f, 0.0f };

		//! キャッシュ用
		XMFLOAT4X4 world{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
		};

		mutable bool dirty = true; //! 変更フラグ

		mutable XMFLOAT3 forward{ 0.0f, 0.0f, 1.0f };	//! 前方向ベクトル(キャッシュ用)
		mutable XMFLOAT3 up{ 0.0f, 1.0f, 0.0f };		//! 上方向ベクトル(キャッシュ用)
		mutable XMFLOAT3 right{ 1.0f, 0.0f, 0.0f };		//! 右方向ベクトル(キャッシュ用)
		mutable bool axesDirty = true;					//! 方向ベクトル更新フラグ

		//! @brief 角度変換ラップ[-180, 180]
		inline float WrapDeg(float _deg)
		{
			float rad = XMConvertToRadians(_deg);
			rad = XMScalarModAngle(rad);
			return XMConvertToDegrees(rad);
		}

		/**
		 * @brief 回転更新(オイラー角度指定、度数法)
		 * @param _eulerDeg 更新後のオイラー角度(度数法)
		 */
		void SetRotationEulerDeg(const XMFLOAT3& _eulerDeg)
		{
			XMFLOAT3 d{
				WrapDeg(_eulerDeg.x),
				WrapDeg(_eulerDeg.y),
				WrapDeg(_eulerDeg.z)
			};
			eulerDegCache = d;

			const float pitch = XMConvertToRadians(d.x);
			const float yaw = XMConvertToRadians(d.y);
			const float roll = XMConvertToRadians(d.z);

			XMVECTOR q = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
			q = XMQuaternionNormalize(q);
			XMStoreFloat4(&rotationQuat, q);

			dirty = true;
			axesDirty = true;
		}

		/**
		 * @brief 現在の角度(度)を取得（UI表示用キャッシュ）
		 */
		XMFLOAT3 GetRotationEulerDeg() const {
			return eulerDegCache;
		}

		/**
		 * @brief
		 */
		void SyncEulerFromQuat() const
		{
			RecalcAxes();
			XMVECTOR f = XMVector3Normalize(XMLoadFloat3(&forward));
			XMVECTOR u = XMVector3Normalize(XMLoadFloat3(&up));
			XMVECTOR r = XMVector3Normalize(XMLoadFloat3(&right));

			const float fx = XMVectorGetX(f);
			const float fy = XMVectorGetY(f);
			const float fz = XMVectorGetZ(f);

			// Yaw: XZ平面での向き
			float yaw = std::atan2f(fx, fz);
			// Pitch: 前ベクトルの上下
			float pitch = std::atan2f(-fy, std::sqrtf(fx * fx + fz * fz));
			// Roll: upとrightで前方向周りの回転を推定
			float roll = 0.0f;
			{
				XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
				float uy = XMVectorGetY(u);
				float ry = XMVectorGetY(r);
				roll = std::atan2f(ry, uy);
			}

			eulerDegCache = {
				XMConvertToDegrees(pitch),
				XMConvertToDegrees(yaw),
				XMConvertToDegrees(roll)
			};
		}

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
		 * @brief 回転更新（クォータニオン直接指定）
		 * @param _dir 更新後の方向
		 */
		void SetRotation(const XMFLOAT4& _dir) {
			XMVECTOR q = XMQuaternionNormalize(XMLoadFloat4(&_dir));
			XMStoreFloat4(&rotationQuat, q);
			dirty = true;
			axesDirty = true;
			// エディター角度が必要な場合は適宜同期
			// SyncEulerFromQuat();
		}
		void AddRotation(const XMFLOAT4& _dir) {
			XMVECTOR r1 = XMLoadFloat4(&rotationQuat);
			XMVECTOR r2 = XMQuaternionNormalize(XMLoadFloat4(&_dir));
			XMVECTOR r = XMQuaternionMultiply(r2, r1);
			r = XMQuaternionNormalize(r);
			XMStoreFloat4(&rotationQuat, r);
			dirty = true;
			axesDirty = true;
			// SyncEulerFromQuat();
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


		/**
		 * @brief 指定方向を向く(+zが_dirを向く)
		 * @param _dir   : 向く方向
		 * @param _upHint: 上方向のヒント(+y)
		 */
		void LookTo(const XMFLOAT3& _dir, const XMFLOAT3& _upHint = XMFLOAT3(0.0f, 1.0f, 0.0f))
		{
			XMVECTOR f = XMLoadFloat3(&_dir);
			if (XMVectorGetX(XMVector3LengthSq(f)) < 1e-8f) { return; }
			f = XMVector3Normalize(f);

			XMVECTOR u = XMLoadFloat3(&_upHint);
			u = XMVector3Normalize(u);

			// forwardがほとんどUpと並行なら
			float dotFU = XMVectorGetX(XMVector3Dot(f, u));
			if (fabsf(dotFU) > 0.98f) {
				// 別軸でも試す
				XMVECTOR alt = XMVectorSet(0, 0, 1, 0);
				if (fabsf(XMVectorGetX(XMVector3Dot(f, alt))) > 0.9f) {
					alt = XMVectorSet(1, 0, 0, 0);
				}
				u = XMVector3Normalize(XMVector3Cross(alt, f));
			}
			else {
				// memo: クロス積を安定させるため直交化させておく
				u = XMVector3Normalize(u - f * XMVectorGetX(XMVector3Dot(f, u)));
			}

			XMMATRIX view = XMMatrixLookToLH(XMVectorZero(), f, u);
			XMVECTOR det{};
			XMMATRIX worldNoTrans = XMMatrixInverse(&det, view);
			XMVECTOR q = XMQuaternionNormalize(XMQuaternionRotationMatrix(worldNoTrans));

			XMStoreFloat4(&rotationQuat, q);
			dirty = true;
			axesDirty = true;
		}

		void LookAt(const XMFLOAT3& _target, const XMFLOAT3& _upHint = XMFLOAT3(0.0f, 1.0f, 0.0f))
		{
			XMFLOAT3 dir{
				_target.x - position.x,
				_target.y - position.y,
				_target.z - position.z,
			};

			LookTo(dir, _upHint);
		}

		/**
		 * @brief +Z を _dir に向ける回転を設定
		 */
		void SetRotationFromDirection(const XMFLOAT3& _dir)
		{
			XMVECTOR v1 = XMLoadFloat3(&_dir);
			if (XMVectorGetX(XMVector3LengthSq(v1)) < 1e-8f) { return; }
			v1 = XMVector3Normalize(v1);

			// 元の forward (+Z)
			XMVECTOR v0 = XMVectorSet(0, 0, 1, 0);

			// From-To Quaternion
			XMVECTOR c = XMVector3Cross(v0, v1);
			float d = XMVectorGetX(XMVector3Dot(v0, v1));

			// v0 と v1 がほぼ正反対のとき
			if (d < -0.9999f) {
				// 任意の直交軸（X軸を使う）
				XMVECTOR axis = XMVectorSet(1, 0, 0, 0);
				XMVECTOR q = XMQuaternionRotationAxis(axis, XM_PI);
				XMStoreFloat4(&rotationQuat, q);
			}
			else {
				XMVECTOR q = XMVectorSet(
					XMVectorGetX(c),
					XMVectorGetY(c),
					XMVectorGetZ(c),
					1.0f + d
				);
				q = XMQuaternionNormalize(q);
				XMStoreFloat4(&rotationQuat, q);
			}

			dirty = true;
			axesDirty = true;
		}


		//! @brief ワールド行列の更新
		void BuildWorld()
		{
			if (!dirty) { return; }

			XMVECTOR s = XMLoadFloat3(&scale);
			XMVECTOR r = XMQuaternionNormalize(XMLoadFloat4(&rotationQuat));
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
		//! @brief Y軸回りの角度取得
		const float GetYaw() const
		{
			const float fx = forward.x;
			const float fz = forward.z;
			if (fabsf(fx) < 1e-6f && fabsf(fz) < 1e-6f) {
				return 0.0f;
			}

			return std::atan2f(fx, fz);
		}

		//! @brief カメラ用のLookTo行列
		const XMMATRIX MakeLookToLH() const
		{
			RecalcAxes();
			XMVECTOR f = XMVector3Normalize(XMLoadFloat3(&forward));
			XMVECTOR u = XMVector3Normalize(XMLoadFloat3(&up));

			// forwardがほとんどUpと並行なら
			float dotFU = XMVectorGetX(XMVector3Dot(f, u));
			if (fabsf(dotFU) > 0.98f) {
				// 別軸でも試す
				XMVECTOR alt = XMVectorSet(0, 0, 1, 0);
				if (fabsf(XMVectorGetX(XMVector3Dot(f, alt))) > 0.9f) {
					alt = XMVectorSet(1, 0, 0, 0);
				}
				u = XMVector3Normalize(XMVector3Cross(alt, f));
			}
			else {
				// memo: クロス積を安定させるため直交化させておく
				u = XMVector3Normalize(u - f * XMVectorGetX(XMVector3Dot(f, u)));
			}
			XMVECTOR eye = XMLoadFloat3(&position);
			return XMMatrixLookToLH(eye, f, u);
		}


		// ---------- World参照 ---------- // 
		XMFLOAT3 GetWorldPosition() const noexcept
		{
			return { world._41, world._42, world._43 };
		};
		XMFLOAT3 GetWorldForward() const noexcept
		{
			return { world._31, world._32, world._33 };
		}
		XMFLOAT3 GetWorldUp() const noexcept
		{
			return { world._21,  world._22, world._23 };
		}
		XMFLOAT3 GetWorldRight() const noexcept
		{
			return { world._11, world._12, world._13 };
		}
		// Vector版
		XMVECTOR GetWorldPositionV() const noexcept
		{
			return XMVectorSet(world._41, world._42, world._43, 1.0f);
		};
		XMVECTOR GetWorldForwardV() const noexcept
		{
			return XMVectorSet(world._31, world._32, world._33, 0.0f);
		}
		XMVECTOR GetWorldUpV() const noexcept
		{
			return XMVectorSet(world._21, world._22, world._23, 0.0f);
		}
		XMVECTOR GetWorldRightV() const noexcept
		{
			return XMVectorSet(world._11, world._12, world._13, 0.0f);
		}
	};
} // namespace ecs

ECS_REFLECT_BEGIN(ecs::Transform)
ECS_REFLECT_FIELD(position),
ECS_REFLECT_FIELD(rotationQuat),
ECS_REFLECT_FIELD(scale)
ECS_REFLECT_END()
