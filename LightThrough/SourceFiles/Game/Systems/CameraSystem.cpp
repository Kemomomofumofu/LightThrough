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
#include <Game/Components/Transform.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>


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
		for (auto& e : entities_) {
			auto& t = ecs_.GetComponent<Transform>(e);
			auto& cam = ecs_.GetComponent<Camera>(e);

			// CameraControllerコンポーネントがあれば更新
			if (ecs_.HasComponent<CameraController>(e)) {
				auto& ctrl = ecs_.GetComponent<CameraController>(e);
				UpdateController(_dt, t, ctrl);
			}

			// view行列の計算(左手系)
			XMVECTOR pos = XMLoadFloat3(&t.position);
			XMVECTOR q = XMLoadFloat4(&t.rotationQuat);
			q = XMQuaternionNormalize(q);

			const XMVECTOR basisForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			const XMVECTOR basisUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

			// forward / up
			XMVECTOR forward = XMVector3Rotate(basisForward, q);
			XMVECTOR up = XMVector3Rotate(basisUp, q);
			
			XMMATRIX viewM = XMMatrixLookAtLH(pos, XMVectorAdd(pos, forward), up);

			// proj行列の計算
			XMMATRIX projM = XMMatrixPerspectiveFovLH(
				XMConvertToRadians(cam.fovY),
				cam.aspectRatio,
				cam.nearZ,
				cam.farZ
			);

			cam.view = viewM;
			cam.proj = projM;
		}
	}

	/**
	 * @brief カメラコントローラーの更新
	 * @param _dt デルタタイム
	 * @param _tf Transformコンポーネント
	 * @param _ctrl CameraControllerコンポーネント
	 */
	void CameraSystem::UpdateController(float _dt, Transform& _transform, CameraController& _controller)
	{
		using namespace DirectX;
		auto& input = input::InputSystem::Get();

		dx3d::Point delta = input.GetMouseDelta();	// 移動量
		const float sensitivity = _controller.mouseSensitivity;		// 感度係数	
		float dx = delta.x * sensitivity;
		float dy = (_controller.invertY ? -delta.y : delta.y) * sensitivity;	// Y軸が反転しているかで変わる

		// FPSモード
		if(_controller.mode == CameraMode::FPS) {
			// ---------- 視点操作 ---------- // 
			_controller.yaw += dx;
			_controller.pitch += dy;
			// ピッチ制限
			_controller.pitch = std::clamp(_controller.pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

			// Yaw/Pitch からクォータニオン生成(Roll = 0)
			XMVECTOR q = XMQuaternionRotationRollPitchYaw(_controller.pitch, _controller.yaw, 0.0f);
			q = XMQuaternionNormalize(q);
			XMStoreFloat4(&_transform.rotationQuat, q);

			// ---------- 移動操作 ---------- // 
			 XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			XMVECTOR forward = XMVectorSet(sinf(_controller.yaw), 0, cosf(_controller.yaw), 0);
			XMVECTOR right = XMVector3Cross(up, forward);

			XMVECTOR pos = XMLoadFloat3(&_transform.position);
			const float moveSpeed = _controller.moveSpeed * _dt;	// 速度係数
			if(input.IsKeyDown('W')) {
				pos += forward * moveSpeed;
			}
			if(input.IsKeyDown('S')) {
				pos -= forward * moveSpeed;
			}
			if(input.IsKeyDown('A')) {
				pos -= right * moveSpeed;
			}
			if(input.IsKeyDown('D')) {
				pos += right * moveSpeed;
			}
			if (input.IsKeyDown('E')) {
				pos += up * moveSpeed;
			}
			if (input.IsKeyDown('Q')) {
				pos -= up * moveSpeed;
			}

			XMStoreFloat3(&_transform.position, pos);
		}
		// Orbitモード
		else {
			// ---------- 視点操作 ---------- // 
			_controller.orbitYaw += dx;
			_controller.orbitPitch += dy;
			// ピッチ制限
			_controller.orbitPitch = std::clamp(_controller.orbitPitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

			// [ToDo] InputSystemにマウスホイールの実装が出来たら距離を変えられるようにする
			//_controller.orbitDistance = std::clamp(_controller.orbitDistance - wheelDelta * 0.5f, 1.0f, 50.0f);

			// 注視点からの相対位置を計算
			XMMATRIX rot = XMMatrixRotationRollPitchYaw(_controller.orbitPitch, _controller.orbitYaw, 0.0f);
			XMVECTOR offset = XMVector3TransformNormal(XMVectorSet(0.0, 0.0f, -_controller.orbitDistance, 0.0f), rot);

			XMVECTOR target = XMLoadFloat3(&_controller.orbitTarget);
			XMVECTOR pos = target + offset;

			XMStoreFloat3(&_transform.position, pos);

			// Transform.rotationに保持しとく
			XMVECTOR forward = XMVector3Normalize(target - pos);
			float yaw = atan2f(XMVectorGetX(forward), XMVectorGetZ(forward));
			float pitch = asinf(-XMVectorGetY(forward));

			_controller.orbitYaw = yaw;
			_controller.orbitPitch = pitch;

			// クォータニオン生成
			XMVECTOR q = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.0f);
			q = XMQuaternionNormalize(q);
			XMStoreFloat4(&_transform.rotationQuat, q);

		}

	}
}