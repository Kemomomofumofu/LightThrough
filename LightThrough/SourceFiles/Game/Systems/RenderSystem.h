#pragma once
/**
 * @file RenderSystem.h
 * @brief 描画システム
 * @author Arima Keita
 * @date 2025-09-01
 */

// ---------- インクルード ---------- // 
#include <DX3D/Game/ECS/ISystem.h>

// ---------- 前方宣言 ---------- //
namespace dx3d {
	class GraphicsEngine;
}

namespace ecs {
	class RenderSystem : public ISystem{
	public:
		void SetGraphicsEngine(dx3d::GraphicsEngine& _engine) { engine_ = &_engine; }

		void Update(float _dt, ecs::Coordinator& _ecs) override;
	private:
		dx3d::GraphicsEngine* engine_;
	};

}