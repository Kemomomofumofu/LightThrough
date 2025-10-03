#pragma once
/**
 * @file TransformSystem.h
 * @brief Transformシステム
 * @author Arima Keita
 * @date 2025-09-04
 */


 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>

namespace ecs {
	/**
	 * @brief Transformシステム
	 * 
	 * Transformコンポーネントのワールド行列を計算する
	 */
	class TransformSystem : public ISystem {
	public:
		TransformSystem(const SystemDesc& _desc);
		void Init();
		void Update(float _dt) override;
	};
}