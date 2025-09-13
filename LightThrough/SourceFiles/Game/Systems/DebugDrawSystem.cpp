/**
 * @file DebugDrawSystem.cpp
 * @brief デバッグ描画システム
 * @author Arima Keita
 * @date 2025-09-12
 */

// ---------- インクルード ---------- // 
#include <Game/Systems/DebugDrawSystem.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/PrimitiveFactory.h>
#include <DX3D/Game/ECS/Coordinator.h>
#include <Game/Components/Camera.h>


namespace ecs {
	// ---------- 名前空間 ---------- // 
	using namespace DirectX;


	void DebugDrawSystem::Init(ecs::Coordinator& _ecs)
	{
		// 初期化
		cb_per_frame_ = engine_->GetGraphicsDevice().CreateConstantBuffer({
			sizeof(dx3d::CBPerFrame),
			nullptr
			});

		cb_per_object_ = engine_->GetGraphicsDevice().CreateConstantBuffer({
			sizeof(dx3d::CBPerObject),
			nullptr
			});

		// Meshを事前に生成しておく
		cube_mesh_ = dx3d::PrimitiveFactory::CreateCube(device_);

	}

	/**
	 * @brief 線描画
	 * @param _start	始点
	 * @param _end		終点
	 * @param _color	色
	 */
	void DebugDrawSystem::DrawLine(XMFLOAT3 _start, XMFLOAT3 _end, XMFLOAT4 _color)
	{

	}


	/**
	 * @brief 立方体描画
	 * @param _pos		座標
	 * @param _size		サイズ
	 * @param _color	色
	 */
	void DebugDrawSystem::DrawCube(XMFLOAT3 _pos, XMFLOAT3 _size, XMFLOAT4 _color)
	{
		DebugCommand cmd;
		cmd.mesh = cube_mesh_;
	}

	/**
	 * @brief 球体描画
	 * @param _pos		座標
	 * @param _radius	半径
	 * @param _color	色
	 */
	void DebugDrawSystem::DrawSphere(XMFLOAT3 _pos, float _radius, XMFLOAT4 _color)
	{

	}

	/**
	 * @brief 更新
	 * @param _dt	デルタタイム
	 * @param _ecs	ecsコーディネータ
	 */
	void DebugDrawSystem::Update(float _dt, Coordinator& _ecs)
	{
		auto& context = engine_->GetDeviceContext();

		// CBPerFrame
		auto camEntity = _ecs.GetEntitiesWithComponent<Camera>()[0];
		auto& cam = _ecs.GetComponent<Camera>(camEntity);

		dx3d::CBPerFrame cbPerFrameData;
		cbPerFrameData.view = cam.view;
		cbPerFrameData.proj = cam.proj;

		// 定数バッファ更新
		cb_per_frame_->Update(context, &cbPerFrameData, sizeof(cbPerFrameData));
		for (auto& cmd : commands_) {
			// ワールド座標行列の取得
			dx3d::CBPerObject cbPerObjectData;
			cbPerObjectData.world = cmd.world;
			cbPerObjectData.color = cmd.color;
			cb_per_object_->Update(context, &cbPerObjectData, sizeof(cbPerObjectData));
			context.VSSetConstantBuffer(1, *cb_per_object_);

			context.SetVertexBuffer(*cmd.mesh.vb);
			context.SetIndexBuffer(*cmd.mesh.ib);

			context.DrawIndexed(cmd.mesh.ib->GetIndexCount(), 0, 0);

		}
		commands_.clear();
	}
}

