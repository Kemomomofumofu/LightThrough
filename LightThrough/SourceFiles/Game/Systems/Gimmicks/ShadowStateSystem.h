#pragma once
/**
 * @file ShadowStateSystem.h
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
	 * @brief 影ステート更新
	 *
	 * 影の中にいるかどうかを判定、更新する
	 * 必須：<Transform>, <Collider>, <ShadowState>
	 */
	class ShadowStateSystem : public ISystem {
	public:
		ShadowStateSystem(const SystemDesc& _desc);
		void Init() override;
		void Update(float _dt) override;

	private:
		void UpdateController(float _dt, Transform& _tf, CameraController& _ctrl);
	};
}
