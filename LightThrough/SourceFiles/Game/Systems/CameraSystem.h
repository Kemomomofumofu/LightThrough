#pragma once
/**
 * @file CameraSystem.h
 * @brief カメラシステム
 * @author Arima Keita
 * @date 2025-09-05
 */

 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>


namespace ecs {	
	class Coordinator;
	struct CameraController;
	struct Transform;
	/**
	 * @brief カメラ
	 *
	 * カメラの挙動
	 * 必須：<Transform>, <Camera>
	 * 任意：<CameraController>
	 */
	class CameraSystem : public ISystem {
	public:
		CameraSystem(const SystemDesc& _desc);
		void Init() override;
		void Update(float _dt) override;

	private:
		void UpdateController(float _dt, Entity _cameraEntity, Transform* _tf, CameraController* _ctrl);
	};
}
