#pragma once
/**
 * @file CameraSystem.h
 * @brief カメラシステム
 * @author Arima Keita
 * @date 2025-09-05
 */

 // ---------- インクルード ---------- //
#include <DX3D/Game/ECS/ISystem.h>


namespace ecs {
	class Coordinator;
	class CameraController;
	class Transform;
	/**
	 * @brief カメラ
	 *
	 * カメラの挙動
	 * 必須：<Transform>, <Camera>
	 * 任意：<CameraController>
	 */
	class CameraSystem : public ISystem {
	public:
		CameraSystem(ecs::Coordinator& _ecs);
		void Init(ecs::Coordinator& _ecs);
		void Update(float _dt, ecs::Coordinator& _ecs);

	private:
		void UpdateController(float _dt, Transform& _tf, CameraController& _ctrl);
	};
}
