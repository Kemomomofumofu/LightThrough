/**
 * @file RenderSystem.cpp
 * @brief 描画システム
 * @author Arima Keita
 * @date 2025-09-01
 */

 // ---------- インクルード ---------- //
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Game/ECS/Coordinator.h>
#include <Game/Systems/RenderSystem.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Mesh.h>

namespace ecs {

	/**
	 * @brief 更新処理
	 * @param _dt デルタタイム
	 * @param _ecs コーディネータ
	 */
	void RenderSystem::Update(float _dt, ecs::Coordinator& _ecs) {
		// 描画開始
		engine_->BeginFrame();

		// 描画
		for (auto& e : entities_) {
			auto& mesh = _ecs.GetComponent<Mesh>(e);
			auto& transform = _ecs.GetComponent<ecs::Transform>(e);

			engine_->Render(*mesh.vb, *mesh.ib);
		}

			//// 頂点バッファ
			//auto& vb = *vb_;
			//context.SetVertexBuffer(vb);
			//context.DrawTriangleList(vb.GetVertexListSize(), 0u);	// テストでの三角形描画

		// 描画終了
		engine_->EndFrame();
	}
}