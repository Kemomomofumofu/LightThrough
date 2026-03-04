#pragma once
/**
 * @file TriggerEventDispatchSystem.h
 */

 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>


namespace ecs {
	
	/**
	 * @brief トリガーイベントを消化するシステム
	 */
	class TriggerEventDispatchSystem : public ISystem
	{
		public:
		explicit TriggerEventDispatchSystem(const SystemDesc& _desc);
		void Init() override;
		void FixedUpdate(float _dt) override;
	};
}