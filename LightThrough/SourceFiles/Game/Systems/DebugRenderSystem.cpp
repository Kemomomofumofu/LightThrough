/**
 * @file DebugDrawSystem.cpp
 * @brief デバッグ描画システム
 * @author Arima Keita
 * @date 2025-09-12
 */

// ---------- インクルード ---------- // 
#include <Game/Systems/DebugRenderSystem.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/PrimitiveFactory.h>
#include <DX3D/Game/ECS/Coordinator.h>
#include <Game/Components/Camera.h>
#include <Game/Components/Transform.h>


namespace ecs {
	// ---------- 名前空間 ---------- // 
	using namespace DirectX;


	DebugRenderSystem::DebugRenderSystem(const dx3d::SystemDesc& _desc)
		: ISystem(_desc)
	{

	}


	/**
	 * @brief 初期化
	 * @param _ecs		ecsコーディネータ
	 */

	void DebugRenderSystem::Init(const ecs::Coordinator& _ecs)
	{
		auto& device = engine_->GetGraphicsDevice();

		// 初期化
		// Meshを事前に生成しておく
		cube_mesh_ = dx3d::PrimitiveFactory::CreateCube(device);
		//sphere_mesh_ = dx3d::PrimitiveFactory::CreateSphere(engine_->GetGraphicsDevice(), 16, 16, 1.0f);
		//line_mesh_ = dx3d::PrimitiveFactory::CreateLine(engine_->GetGraphicsDevice(), {0,0,0}, {1,0,0});

		cb_per_frame_ = device.CreateConstantBuffer({
			sizeof(dx3d::CBPerFrame),
			nullptr
			});

		cb_per_object_ = device.CreateConstantBuffer({
			sizeof(dx3d::CBPerObject),
			nullptr
			});


	}

	/**
	 * @brief 線描画
	 * @param _start	始点
	 * @param _end		終点
	 * @param _color	色
	 */
	void DebugRenderSystem::DrawLine(XMFLOAT3 _start, XMFLOAT3 _end, XMFLOAT4 _color)
	{

	}


	/**
	 * @brief 立方体描画
	 * @param _transform	transform
	 * @param _color		色
	 */
	void DebugRenderSystem::DrawCube(const Transform& _transform, XMFLOAT4 _color)
	{
		DebugCommand cmd;
		cmd.mesh = cube_mesh_;
		cmd.world = 
			XMMatrixScaling(_transform.scale.x, _transform.scale.y, _transform.scale.z) *
			XMMatrixRotationRollPitchYaw(_transform.rotation.x, _transform.rotation.y, _transform.rotation.z) *
			XMMatrixTranslation(_transform.position.x, _transform.position.y, _transform.position.z);
		cmd.color = _color;
		commands_.emplace_back(std::move(cmd));
	}

	/**
	 * @brief 球体描画
	 * @param _pos		座標
	 * @param _color	色
	 */
	void DebugRenderSystem::DrawSphere(const Transform& _transform, XMFLOAT4 _color)
	{
		// [ToDo] 半径はscale.x / 2 としておく
		float radius = _transform.scale.x * 0.5f;

		DebugCommand cmd;
		cmd.mesh = sphere_mesh_;
		cmd.world =
			XMMatrixScaling(radius, radius, radius) *
			XMMatrixTranslation(_transform.position.x, _transform.position.y, _transform.position.z);
		cmd.color = _color;
		commands_.emplace_back(std::move(cmd));
	}

	/**
	 * @brief 更新
	 * @param _dt	デルタタイム
	 * @param _ecs	ecsコーディネータ
	 */
	void DebugRenderSystem::Update(float _dt, Coordinator& _ecs)
	{
		auto& context = engine_->GetDeviceContext();

		// カメラ取得 [ToDo] 現状カメラは一つだけを想定
		auto camEntity = _ecs.GetEntitiesWithComponent<Camera>()[0];
		auto& cam = _ecs.GetComponent<Camera>(camEntity);

		// 定数バッファ更新
		// フレーム単位の定数バッファ更新
		dx3d::CBPerFrame cbPerFrameData{};
		cbPerFrameData.view = cam.view;
		cbPerFrameData.proj = cam.proj;

		cb_per_frame_->Update(context, &cbPerFrameData, sizeof(cbPerFrameData));
		context.VSSetConstantBuffer(0, *cb_per_frame_);

		for (auto& cmd : commands_) {
			// ワールド座標行列の取得
			// オブジェクト単位の定数バッファ更新
			dx3d::CBPerObject cbPerObjectData{};
			cbPerObjectData.world = cmd.world;
			cbPerObjectData.color = cmd.color;
			cb_per_object_->Update(context, &cbPerObjectData, sizeof(cbPerObjectData));
			context.VSSetConstantBuffer(1, *cb_per_object_);

			engine_->Render(*cmd.mesh.vb, *cmd.mesh.ib);



		}
		commands_.clear();
	}
}

