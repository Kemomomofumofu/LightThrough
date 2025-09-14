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

		//
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
}