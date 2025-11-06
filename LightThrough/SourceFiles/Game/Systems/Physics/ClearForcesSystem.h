#pragma once

/**
 * @file ClearForcesSystem.h
 * @brief フレームの末で外力・トルクをリセット
 */


 // ---------- インクルード ---------- // 
#include <Game/ECS/ISystem.h>


namespace ecs {
	class ClearForcesSystem :
		public ISystem
	{
		public:
		explicit ClearForcesSystem(const SystemDesc& _desc)
			: ISystem(_desc) {}
		void Init();
		void FixedUpdate(float _fixedDt) override;
	};
}