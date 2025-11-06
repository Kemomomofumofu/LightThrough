#pragma once
/**
 * @file ForceAccumulationSystem.h
 * @brief AddForceやAddTorqueを反映するシステム
 */



 // ---------- インクルード ---------- // 
#include <Game/ECS/ISystem.h>

namespace ecs {
	class ForceAccumulationSystem : public ISystem
	{
	public:
		explicit ForceAccumulationSystem(const SystemDesc& _desc)
			: ISystem(_desc) {}
		
		void Init();
		void FixedUpdate(float _fixedDt) override;

	private:
		float gravity_ = -9.81f;

	};
}