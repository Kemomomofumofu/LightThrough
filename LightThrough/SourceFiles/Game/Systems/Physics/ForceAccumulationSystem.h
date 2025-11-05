#pragma once
/**
 * @file ForceAccumulationSystem.h
 * @brief AddForce‚âAddTorque‚ð”½‰f‚·‚éƒVƒXƒeƒ€
 */



#include <Game/ECS/ISystem.h>

namespace ecs {

	class ForceAccumulationSystem : public ISystem
	{
	public:
		explicit ForceAccumulationSystem(const SystemDesc& _desc);
		
		void Init();
		void FixedUpdate(float _fixedDt) override;

	private:
		float gravity_ = -9.81f;

	};
}