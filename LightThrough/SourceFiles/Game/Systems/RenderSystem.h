#pragma once
/**
 * @file RenderSystem.h
 * @brief 描画システム
 * @author Arima Keita
 * @date 2025-09-01
 */

// ---------- インクルード ---------- // 
#include <DX3D/Core/Core.h>
#include <DX3D/Graphics/Buffers/ConstantBuffer.h>
#include <DX3D/Game/ECS/ISystem.h>

// ---------- 前方宣言 ---------- //
namespace dx3d {
	class GraphicsEngine;
}

namespace ecs {

	/**
	 * @brief 描画システム
	 *
	 * 必須；<Transform>, <Mesh>
	 */
	class RenderSystem : public ISystem{
	public:
		RenderSystem(const dx3d::SystemDesc& _desc);
		void Init(ecs::Coordinator& _ecs);
		void SetGraphicsEngine(dx3d::GraphicsEngine& _engine) { engine_ = &_engine; }

		void Update(float _dt, ecs::Coordinator& _ecs) override;
	private:
		dx3d::GraphicsEngine* engine_{};
		dx3d::ConstantBufferPtr cb_per_frame_{};
		dx3d::ConstantBufferPtr cb_per_object_{};
	};

}