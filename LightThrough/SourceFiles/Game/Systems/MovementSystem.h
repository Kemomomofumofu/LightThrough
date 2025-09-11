#pragma once

/**
 * @file MovementSystem.h
 * @brief 移動システム
 * @author Arima Keita
 * @date 2025-08-17
 */

 // ---------- インクルード ---------- //
#include <DX3D/Game/ECS/ISystem.h>

namespace ecs {
	class Coordinator;
	/**
	 * @brief 移動システム
	 *
	 * 物体の位置を更新するシステム。
	 * <Transform>, <Velocity>
	 */
	class MovementSystem : public ISystem {
	public:
		MovementSystem(ecs::Coordinator& _ecs);
		void Update(float _dt, ecs::Coordinator& _ecs);
	};
}
