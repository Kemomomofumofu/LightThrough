#pragma once

/**
 * @file MovementSystem.h
 * @brief 移動システム
 * @author Arima Keita
 * @date 2025-08-17
 */

 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>

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
		MovementSystem(SystemDesc _desc);
		void Update(float _dt);
	};
}
