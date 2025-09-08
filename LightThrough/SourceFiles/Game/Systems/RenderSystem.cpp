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
#include <DX3D/Graphics/ConstantBuffer.h>
#include <DX3D/Game/ECS/Coordinator.h>
#include <Game/Systems/RenderSystem.h>
#include <Game/Components/Camera.h>
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

		// CameraComponentを持つEntityを取得 [ToDo] 現状カメラは一つだけを想定
		auto camEntity = _ecs.GetEntitiesWithComponent<Camera>()[0];	// とりあえず一番最初のカメラを取得しとく
		auto& cam = _ecs.GetComponent<Camera>(camEntity);
		auto& context = engine_->GetDeviceContext();
		
		dx3d::CBPerFrame cbPerFrameData;
		cbPerFrameData.view = cam.view;
		cbPerFrameData.proj = cam.proj;
		// cb_per_frame_.Update(context, cbPerFrameData);
		

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