#pragma once


// ---------- インクルード ---------- // 
#include <Game/ECS/ISystem.h>

namespace ecs {
	class PlayerControllerSystem : public ISystem
	{
	public:
		explicit PlayerControllerSystem(const ecs::SystemDesc& _desc)
			: ISystem(_desc) {}

		void Init();
		void Update(float _dt) override;
		void FixedUpdate(float _fixedDt) override;

	private:
		// 移動フラグ
		bool move_forward_	= false;
		bool move_back_		= false;
		bool move_left_		= false;
		bool move_right_	= false;

	};
} // namespace ecs