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
		// ---------- 前方宣言 ---------- //
	struct Entity;
	class Coordinator;
	class TitleSceneSystem : public ISystem {
	public:
		explicit TitleSceneSystem(const SystemDesc& _desc);
		virtual void Update(float _dt) override;
	};
}


