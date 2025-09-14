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
#include <InputSystem/InputSystem.h>
#include <DX3D/Game/ECS/Coordinator.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>


namespace ecs {
	CameraSystem::CameraSystem(const dx3d::SystemDesc& _desc)
		: ISystem(_desc)
	{
	}

	void CameraSystem::Init(ecs::Coordinator& _ecs)
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(_ecs.GetComponentType<Transform>());
		signature.set(_ecs.GetComponentType<Camera>());
		_ecs.SetSystemSignature<CameraSystem>(signature);
	}

	/**
	 * @brief 更新処理
	 * @param _dt デルタタイム
	 * @param _ecs ECSのコーディネーター
	 */
	void CameraSystem::Update(float _dt, ecs::Coordinator& _ecs)
	{
		using namespace DirectX;
		for (auto& e : entities_) {
			auto& t = _ecs.GetComponent<Transform>(e);
			auto& cam = _ecs.GetComponent<Camera>(e);

			// CameraControllerコンポーネントがあれば更新
			if (_ecs.HasComponent<CameraController>(e)) {
				auto& ctrl = _ecs.GetComponent<CameraController>(e);
				UpdateController(_dt, t, ctrl);
			}

			// view行列の計算(左手系)
			XMVECTOR pos = XMLoadFloat3(&t.position);

			// forwardベクトルを計算
			XMMATRIX rot = XMMatrixRotationRollPitchYaw(t.rotation.x, t.rotation.y, t.rotation.z);
			XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rot);
			XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

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
	void CameraSystem::UpdateController(float _dt, Transform& _tf, CameraController& _ctrl)
	{
		using namespace DirectX;
		auto& input = input::InputSystem::Get();
		dx3d::Point delta = input.GetMouseDelta();	// 移動量
		const float sensitivity = _ctrl.mouseSensitivity * _dt;		// 感度係数	
		float dx = delta.x * sensitivity;
		float dy = (_ctrl.invertY ? -delta.y : delta.y) * sensitivity;	// Y軸が反転しているかで変わる

		// FPSモード
		if(_ctrl.mode == CameraMode::FPS) {
			// ---------- 視点操作 ---------- // 
			_ctrl.yaw += dx;
			_ctrl.pitch += dy;	
			// ピッチ制限
			_ctrl.pitch = std::clamp(_ctrl.pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

			_tf.rotation.x = _ctrl.pitch;
			_tf.rotation.y = _ctrl.yaw;

			// ---------- 移動操作 ---------- // 
			XMVECTOR forward = XMVectorSet(sinf(_ctrl.yaw), 0, cosf(_ctrl.yaw), 0);
			XMVECTOR right = XMVector3Cross(XMVectorSet(0, 1, 0, 0), forward);

			XMVECTOR pos = XMLoadFloat3(&_tf.position);
			const float moveSpeed = _ctrl.moveSpeed * _dt;	// 速度係数
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

			XMStoreFloat3(&_tf.position, pos);
		}
		// Orbitモード
		else {
			// ---------- 視点操作 ---------- // 
			_ctrl.orbitYaw += dx;
			_ctrl.orbitPitch += dy;
			// ピッチ制限
			_ctrl.orbitPitch = std::clamp(_ctrl.pitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);

			// [ToDo] InputSystemにマウスホイールの実装が出来たら距離を変えられるようにする
			//_ctrl.orbitDistance = std::clamp(_ctrl.orbitDistance - wheelDelta * 0.5f, 1.0f, 50.0f);

			// 注視点からの相対位置を計算
			XMMATRIX rot = XMMatrixRotationRollPitchYaw(_ctrl.orbitPitch, _ctrl.orbitYaw, 0.0f);
			XMVECTOR offset = XMVector3TransformNormal(XMVectorSet(0.0, 0.0f, -_ctrl.orbitDistance, 0.0f), rot);

			XMVECTOR target = XMLoadFloat3(&_ctrl.orbitTarget);
			XMVECTOR pos = target + offset;

			XMStoreFloat3(&_tf.position, pos);

			// Transform.rotationに保持しとく
			XMVECTOR forward = XMVector3Normalize(target - pos);
			float yaw = atan2f(XMVectorGetX(forward), XMVectorGetZ(forward));
			float pitch = asinf(-XMVectorGetY(forward));

			_tf.rotation.x = pitch;
			_tf.rotation.y = yaw;
			_tf.rotation.z = 0.0f;
		}

	}
}