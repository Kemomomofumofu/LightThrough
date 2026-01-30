/**
 * @file CameraSystem.cpp
 * @brief カメラシステム
 * @author Arima Keita
 * @date 2025-09-05
 */

 // ---------- インクルード ---------- //
#include <algorithm>
#include <DirectXMath.h>
#include <Game/Systems/CameraSystem.h>
#include <Game/InputSystem/InputSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Components/Core/Transform.h>
#include <Game/Components/Camera/Camera.h>
#include <Game/Components/Input/CameraController.h>
#include <Game/Components/Core/ObjectChild.h>


namespace ecs {

	CameraSystem::CameraSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
	}

	void CameraSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<Camera>());
		ecs_.SetSystemSignature<CameraSystem>(signature);
	}

	/**
	 * @brief 更新処理
	 * @param _dt デルタタイム
	 */
	void CameraSystem::Update(float _dt)
	{
		using namespace DirectX;
		for (auto& camera : entities_) {
			auto tf = ecs_.GetComponent<Transform>(camera);
			auto cam = ecs_.GetComponent<Camera>(camera);

			// CameraControllerコンポーネントがあれば更新
			if (ecs_.HasComponent<CameraController>(camera)) {
				auto ctrl = ecs_.GetComponent<CameraController>(camera);
				UpdateController(_dt, camera, tf, ctrl);
			}

			const XMMATRIX worldM = XMLoadFloat4x4(&tf->world);

			// view / proj 作成
			XMMATRIX viewM = XMMatrixInverse(nullptr, worldM);
			XMMATRIX projM = XMMatrixPerspectiveFovLH(
				XMConvertToRadians(cam->fovY),
				cam->aspectRatio,
				cam->nearZ,
				cam->farZ
			);

			XMStoreFloat4x4(&cam->view, viewM);
			XMStoreFloat4x4(&cam->proj, projM);
		}
	}

	/**
	 * @brief カメラコントローラーの更新
	 * @param _dt デルタタイム
	 * @param _cameraEntity カメラインスタンスのEntity
	 * @param _tf Transformコンポーネント
	 * @param _ctrl CameraControllerコンポーネント
	 */
	void CameraSystem::UpdateController(float _dt, Entity _cameraEntity, Transform* _transform, CameraController* _controller)
	{
		using namespace DirectX;
		auto& input = input::InputSystem::Get();

		// 親の情報取得
		XMMATRIX parentWorld = XMMatrixIdentity();
		XMVECTOR parentRotQ = XMVectorSet(0, 0, 0, 1);
		bool hasParent = false;
		// 親がいるなら
		if (ecs_.HasComponent<ObjectChild>(_cameraEntity)) {
			auto child = ecs_.GetComponent<ObjectChild>(_cameraEntity);
			if(child->root.IsInitialized()) {
				hasParent = true;
				// 親のワールド行列取得
				const auto parentTf = ecs_.GetComponent<Transform>(child->root);
				parentWorld = XMLoadFloat4x4(&parentTf->world);

				// 回転の取得
				XMVECTOR scale, rot, trans;
				XMMatrixDecompose(&scale, &rot, &trans, parentWorld);
				parentRotQ = rot;
			}
		}

		dx3d::Point delta = input.GetMouseDelta();	// マウス差分
		const float sensitivity = _controller->mouseSensitivity;		// 感度係数	
		float dx = delta.x * sensitivity;
		float dy = (_controller->invertY ? -delta.y : delta.y) * sensitivity;	// Y軸が反転しているかで変わる

		// FPSモード
		if (_controller->mode == CameraMode::FPS) {
			// ---------- 視点操作 ---------- // 
			_controller->yaw += dx;
			_controller->pitch += dy;
			// ピッチ制限
			_controller->pitch = std::clamp(_controller->pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

			// 回転
			XMVECTOR localQ = XMQuaternionRotationRollPitchYaw(_controller->pitch, _controller->yaw, 0.0f);
			localQ = XMQuaternionNormalize(localQ);
			XMFLOAT4 qf; XMStoreFloat4(&qf, localQ);
			_transform->SetRotation(qf);

			// ---------- 移動操作 ---------- // 
			if (_controller->enableMove) {
				XMVECTOR forward = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), localQ);
				XMVECTOR right = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), localQ);
				XMVECTOR up = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), localQ);

				XMVECTOR pos = XMLoadFloat3(&_transform->position);
				const float moveSpeed = _controller->moveSpeed * _dt;	// 速度係数
				if (input.IsKeyDown('W')) { pos += forward * moveSpeed; }
				if (input.IsKeyDown('S')) { pos -= forward * moveSpeed; }
				if (input.IsKeyDown('A')) { pos -= right * moveSpeed; }
				if (input.IsKeyDown('D')) { pos += right * moveSpeed; }
				if (input.IsKeyDown('E')) { pos += up * moveSpeed; }
				if (input.IsKeyDown('Q')) { pos -= up * moveSpeed; }

				// 移動
				XMFLOAT3 pf; XMStoreFloat3(&pf, pos);
				_transform->SetPosition(pf);
			}
		}
		// Orbitモード
		// memo: 現状使う予定はなし
		else {
			// orbitTarget はワールド座標（既存のまま）とする想定
			XMMATRIX rot = XMMatrixRotationRollPitchYaw(_controller->orbitPitch, _controller->orbitYaw, 0.0f);
			XMVECTOR offsetWorld = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, -_controller->orbitDistance, 0.0f), rot);
			XMVECTOR targetWorld = XMLoadFloat3(&_controller->orbitTarget);
			XMVECTOR worldPos = targetWorld + offsetWorld;

			if (hasParent) {
				// 親ワールドの逆行列でワールド位置→親ローカル位置に変換
				XMMATRIX invParent = XMMatrixInverse(nullptr, parentWorld);
				XMVECTOR localPosV = XMVector3TransformCoord(worldPos, invParent);
				XMFLOAT3 pf; XMStoreFloat3(&pf, localPosV);
				_transform->SetPosition(pf);

				// ワールド回転 -> ローカル回転（localQ = inverse(parentRot) * worldQ）
				XMVECTOR worldForward = XMVector3Normalize(targetWorld - worldPos);
				float yaw = atan2f(XMVectorGetX(worldForward), XMVectorGetZ(worldForward));
				float pitch = asinf(-XMVectorGetY(worldForward));
				XMVECTOR worldQ = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.0f);
				XMVECTOR localQ = XMQuaternionMultiply(XMQuaternionInverse(parentRotQ), worldQ);
				localQ = XMQuaternionNormalize(localQ);
				XMFLOAT4 qf; XMStoreFloat4(&qf, localQ);
				_transform->SetRotation(qf);
			}
			else {
				// 親がいないならいままで通り世界位置をそのまま設定
				XMFLOAT3 pf; XMStoreFloat3(&pf, worldPos);
				_transform->SetPosition(pf);
				XMVECTOR worldForward = XMVector3Normalize(targetWorld - worldPos);
				float yaw = atan2f(XMVectorGetX(worldForward), XMVectorGetZ(worldForward));
				float pitch = asinf(-XMVectorGetY(worldForward));
				XMVECTOR q = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.0f);
				q = XMQuaternionNormalize(q);
				XMFLOAT4 qf; XMStoreFloat4(&qf, q);
				_transform->SetRotation(qf);
			}
		}
	}
}