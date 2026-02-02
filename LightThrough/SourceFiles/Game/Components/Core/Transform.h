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

#include <DX3D/Math/MathUtils.h>

namespace ecs {
	using namespace DirectX;

	struct Transform {
		XMFLOAT3 position = { 0, 0, 0 };					// 位置
		XMFLOAT4 rotationQuat = { 0, 0, 0, 1 };				// 回転(クォータニオン)
		XMFLOAT3 scale = { 1, 1, 1 };						// スケール

		// ---------- キャッシュ ---------- // 
		// ワールド
		XMFLOAT4X4 world{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
		};
		// 物理向けキャッシュ
		mutable XMFLOAT3 worldForward{ 0.0f, 0.0f, 1.0f };
		mutable XMFLOAT3 worldUp{ 0.0f, 1.0f, 0.0f };
		mutable XMFLOAT3 worldRight{ 1.0f, 0.0f, 0.0f };
		mutable XMFLOAT3 worldScale{ 1.0f, 1.0f, 1.0f };
		mutable XMFLOAT4 worldRotationQuat{ 0, 0, 0, 1 };
		// ローカル軸
		mutable XMFLOAT3 forward{ 0.0f, 0.0f, 1.0f };
		mutable XMFLOAT3 up{ 0.0f, 1.0f, 0.0f };
		mutable XMFLOAT3 right{ 1.0f, 0.0f, 0.0f };

		// ---------- フラグ ---------- //
		mutable bool dirty = true;		// ローカル変更フラグ
		mutable bool axesDirty = true;	// 方向ベクトル更新フラグ
		mutable bool worldDirty = true; // ワールド行列更新フラグ

		// エディター用
		// オイラー角度キャッシュ(度数法)
		mutable XMFLOAT3 eulerDegCache{ 0.0f, 0.0f, 0.0f };

		//! @brief 角度変換ラップ[-180, 180]
		inline float WrapDeg(float _deg)
		{
			float rad = XMConvertToRadians(_deg);
			rad = XMScalarModAngle(rad);
			return XMConvertToDegrees(rad);
		}


		// ---------- 計算関連 ---------- //
		/**
		 * @brief ローカル軸再計算
		 */
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

		/**
		 * @brief 指定位置を向く(+zが_targetを向く)
		 * @param _target : 向く位置
		 * @param _upHint : 上方向のヒント(+y)
		 */
		void LookAt(const XMFLOAT3& _target, const XMFLOAT3& _upHint = XMFLOAT3(0.0f, 1.0f, 0.0f))
		{
			auto dir = math::Sub(_target, position);
			LookTo(dir, _upHint);
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


		// ---------- Setter, Getter ---------- // 
		// memo: セッターを介して変更することで自動的にDirtyフラグを立てる

		/**
		 * @brief 位置更新
		 */
		void SetPosition(const XMFLOAT3& _pos) {
			position = _pos;
			dirty = true;
			worldDirty = true;
		}
		void AddPosition(const XMFLOAT3& _pos) {
			position = math::Add(position, _pos);
			dirty = true;
			worldDirty = true;
		}

		/**
		 * @brief 回転更新（クォータニオン直接指定）
		 */
		void SetRotation(const XMFLOAT4& _dir) {
			XMVECTOR q = XMQuaternionNormalize(XMLoadFloat4(&_dir));
			XMStoreFloat4(&rotationQuat, q);
			dirty = true;
			axesDirty = true;
			worldDirty = true;
		}
		void AddRotation(const XMFLOAT4& _dir) {
			XMVECTOR r1 = XMLoadFloat4(&rotationQuat);
			XMVECTOR r2 = XMQuaternionNormalize(XMLoadFloat4(&_dir));
			XMVECTOR r = XMQuaternionMultiply(r2, r1);
			r = XMQuaternionNormalize(r);
			XMStoreFloat4(&rotationQuat, r);
			dirty = true;
			axesDirty = true;
			worldDirty = true;
		}

		/**
		 * @brief スケール更新
		 */
		void SetScale(const XMFLOAT3& _scale) {
			scale = _scale;
			dirty = true;
			worldDirty = true;
		}
		void AddScale(const XMFLOAT3& _scale) {
			scale = math::Add(scale, _scale);
			dirty = true;
			worldDirty = true;
		}

		const XMFLOAT3& GetForward() const
		{
			RecalcAxes();
			return forward;
		}
		const XMFLOAT3& GetRight() const
		{
			RecalcAxes();
			return right;
		}
		const XMFLOAT3& GetUp() const
		{
			RecalcAxes();
			return up;
		}

		/**
		 * @brief Yaw角取得（ラジアン）
		 */
		const float GetYaw() const
		{
			RecalcAxes();
			const float fx = forward.x;
			const float fz = forward.z;
			if (fabsf(fx) < 1e-6f && fabsf(fz) < 1e-6f) {
				return 0.0f;
			}

			return std::atan2f(fx, fz);
		}


		// 物理向けキャッシュの取得
		XMFLOAT3 GetWorldRightCached() const noexcept
		{
			return worldRight;
		}
		XMFLOAT3 GetWorldUpCached() const noexcept
		{
			return worldUp;
		}
		XMFLOAT3 GetWorldForwardCached() const noexcept
		{
			return worldForward;
		}
		XMFLOAT3 GetWorldScaleCached() const noexcept
		{
			return worldScale;
		}
		XMFLOAT4 GetWorldRotationQuatCached() const noexcept
		{
			return worldRotationQuat;
		}

		XMFLOAT3 GetWorldPosition() const noexcept
		{
			return XMFLOAT3(
				world._41,
				world._42,
				world._43
			);
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

		// Vector版
		// todo: worldPosもキャッシュする形にしたい。
		XMVECTOR GetWorldPositionV() const noexcept
		{
			return XMVectorSet(world._41, world._42, world._43, 1.0f);
		};



		// ---------- エディター・デバッグ関連 ---------- //
		/**
		 * @brief 現在の角度(度)を取得
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
			worldDirty = true;
		}
	};
} // namespace ecs

ECS_REFLECT_BEGIN(ecs::Transform)
ECS_REFLECT_FIELD(position),
ECS_REFLECT_FIELD(rotationQuat),
ECS_REFLECT_FIELD(scale)
ECS_REFLECT_END()


