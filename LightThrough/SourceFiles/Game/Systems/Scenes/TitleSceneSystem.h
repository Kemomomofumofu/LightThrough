#pragma once
/**
 * @file TitleSceneSystem.h
 * @brief タイトルシーンのふるまい
 * @author Arima Keita
 * @date 2025-09-22
 */

 /*---------- インクルード ----------*/
#include <Game/ECS/ISystem.h>

namespace ecs {
	class TitleSceneSystem : public ISystem {
	public:
		explicit TitleSceneSystem(const SystemDesc& _desc);

		virtual void Init() override;
		virtual void Update(float _dt) override;

		void DebugTitleScene();

	private:
		float time_sec_ = 0.0f;


	};
}


