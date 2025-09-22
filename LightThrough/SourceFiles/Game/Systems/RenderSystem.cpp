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
#include <Game/ECS/Coordinator.h>
#include <Game/Systems/RenderSystem.h>

#include <Game/Components/Camera.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Mesh.h>


namespace ecs {
	/**
	 * @brief コンストラクタ
	 */
	RenderSystem::RenderSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
	}

	void RenderSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<Mesh>());
		ecs_.SetSystemSignature<RenderSystem>(signature);

		// 初期化
		cb_per_frame_ = engine_->GetGraphicsDevice().CreateConstantBuffer({
			sizeof(dx3d::CBPerFrame),
			nullptr
			});

		cb_per_object_ = engine_->GetGraphicsDevice().CreateConstantBuffer({
			sizeof(dx3d::CBPerObject),
			nullptr
			});

	}

	/**
	 * @brief 更新処理
	 * @param _dt デルタタイム
	 */
	void RenderSystem::Update(float _dt)
	{
		auto& context = engine_->GetDeviceContext();
		auto& device = engine_->GetGraphicsDevice();


		// CameraComponentを持つEntityを取得 [ToDo] 現状カメラは一つだけを想定
		auto camEntity = ecs_.GetEntitiesWithComponent<Camera>()[0];	// とりあえず一番最初のカメラを取得しとく
		auto& cam = ecs_.GetComponent<Camera>(camEntity);

		
		dx3d::CBPerFrame cbPerFrameData{};
		cbPerFrameData.view = cam.view;
		cbPerFrameData.proj = cam.proj;
		
		// 定数バッファ更新
		cb_per_frame_->Update(context, &cbPerFrameData, sizeof(cbPerFrameData));
		context.VSSetConstantBuffer(0, *cb_per_frame_);	// 頂点シェーダーのスロット0にセット

		// 描画
		for (auto& e : entities_) {
			auto& mesh = ecs_.GetComponent<Mesh>(e);
			auto& transform = ecs_.GetComponent<ecs::Transform>(e);
			
			// ワールド座標行列の取得
			dx3d::CBPerObject cbPerObjectData{};
			cbPerObjectData.world = transform.GetWorldMatrix();
			cb_per_object_->Update(context, &cbPerObjectData, sizeof(cbPerObjectData));
			context.VSSetConstantBuffer(1, *cb_per_object_);


			engine_->Render(*mesh.vb, *mesh.ib);
		}
	}
}