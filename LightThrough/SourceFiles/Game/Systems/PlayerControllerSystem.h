#pragma once


// ---------- インクルード ---------- // 
#include <Game/ECS/ISystem.h>

namespace ecs {
	class PlayerControllerSystem : public ISystem
	{
	public:
		explicit PlayerControllerSystem(ecs::SystemDesc& _desc)
			: ISystem(_desc) {}

		void Init();
		void Update(float _dt) override;
		void FixedUpdate(float _fixedDt) override;

	private:


	};
} // namespace ecs