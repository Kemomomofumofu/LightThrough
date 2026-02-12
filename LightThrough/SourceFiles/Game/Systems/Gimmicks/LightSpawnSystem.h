#pragma once
/**
 * @file LightSpawnSystem.h
 */

 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>

namespace ecs
{
	/**
	 * @brief ライト設置システム
	 */
	class LightSpawnSystem : public ISystem {
	public:
		explicit LightSpawnSystem(const SystemDesc& _desc);
		//! @brief 初期化
		void Init() override;
		//! @brief 固定更新
		void FixedUpdate(float _fixedDt) override;
	};
}