/**
 * @file CameraSystem.cpp
 * @brief カメラシステム
 * @author Arima Keita
 * @date 2025-09-05
 */

// ---------- インクルード ---------- // 
#include <Game/Systems/CameraSystem.h>
#include <DX3D/Game/ECS/Coordinator.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>


/**
 * @brief 更新処理
 * @param _dt デルタタイム
 * @param _ecs ECSのコーディネーター
 */

void ecs::CameraSystem::Update(float _dt, ecs::Coordinator& _ecs)
{
	using namespace DirectX;
	for (auto& e : entities_) {
		auto& tf = _ecs.GetComponent<Transform>(e);
		auto& cam = _ecs.GetComponent<Camera>(e);

		// CameraControllerコンポーネントがあれば更新
		if (_ecs.HasComponent<ecs::CameraController>(e)) {
			auto& ctrl = _ecs.GetComponent<CameraController>(e);
			UpdateController(_dt, tf, ctrl);
		}

		// view行列の計算(左手系)
		XMVECTOR pos = XMLoadFloat3(&tf.position);

		// forwardベクトルを計算
		XMMATRIX rot = XMMatrixRotationRollPitchYaw(tf.rotation.x, tf.rotation.y, tf.rotation.z);
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
void ecs::CameraSystem::UpdateController(float _dt, ecs::Transform& _tf, ecs::CameraController& _ctrl)
{
	using namespace DirectX;

	if (_ctrl.mode == CameraMode::FPS) {
		_tf.rotation.x = _ctrl.pitch;
		_tf.rotation.y = _ctrl.yaw;
	}
	else {
		XMMATRIX rot = XMMatrixRotationRollPitchYaw(_ctrl.orbitPitch, _ctrl.orbitYaw, 0.0f);
		XMVECTOR dir = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rot);
		XMVECTOR pos = XMLoadFloat3(&_ctrl.orbitTarget) + (-_ctrl.orbitDistance) * dir;
		XMFLOAT3 p; XMStoreFloat3(&p, pos);
		_tf.position = p;
	}
}
