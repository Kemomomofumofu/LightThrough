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
#include <DX3D/Game/ECS/Systems/RenderSystem.h>
#include <DX3D/Game/ECS/Components/Transform.h>
namespace ecs {

	/**
	 * @brief 更新処理
	 * @param _dt デルタタイム
	 * @param _ecs コーディネータ
	 */
	void RenderSystem::Update(float _dt, ecs::Coordinator& _ecs) {
		// 描画開始
		engine_->BeginFrame();

		auto& context = engine_->GetDeviceContext();
		auto& device = engine_->GetGraphicsDevice();
		// 描画
		for (auto& e : entities_) {
			//auto& mesh = _ecs.GetComponent<MeshComponent>(e);
			auto& transform = _ecs.GetComponent<ecs::Transform>(e);

			//context.SetVertexBuffer(*mesh.vb_);
			//context.SetIndexBuffer(*mesh.ib_);
			//context.SetConstantBuffer(transform);
				
			device.ExecuteCommandList(context);
		}

			//// 頂点バッファ
			//auto& vb = *vb_;
			//context.SetVertexBuffer(vb);
			//context.DrawTriangleList(vb.GetVertexListSize(), 0u);	// テストでの三角形描画

		// 描画終了
		engine_->EndFrame();
	}
}