#pragma once
/**
 * @file IntegrationSystem.h
 * @brief 位置更新
 */

 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>

namespace ecs {
	class IntegrationSystem : public ISystem
	{
	public:
		explicit IntegrationSystem(const SystemDesc& _desc)
			: ISystem(_desc) {}

		void Init();
		void FixedUpdate(float _fixedDt) override;
	};
}