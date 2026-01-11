/**
 * @file DebugDrawSystem.cpp
 * @brief デバッグ描画システム
 * @author Arima Keita
 * @date 2025-09-12
 */

 // ---------- インクルード ---------- // 
#include <Game/Systems/Renderers/DebugRenderSystem.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/Meshes/MeshRegistry.h>
#include <DX3D/Graphics/Meshes/Mesh.h>
#include <Game/ECS/Coordinator.h>
#include <DX3D/Graphics/PipelineKey.h>

#include <Game/Components/Camera.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Collider.h>

#include <Game/Collisions/CollisionUtils.h>
#include <Debug/DebugUI.h>

namespace {
	struct CBPerFrame {
		DirectX::XMMATRIX view;	// ビュー行列
		DirectX::XMMATRIX proj;	// プロジェクション行列
	};

	struct CBPerObject {
		DirectX::XMMATRIX world;	// ワールド行列
		DirectX::XMFLOAT4 color;	// 色
	};
}


namespace ecs {
	// ---------- 名前空間 ---------- // 
	using namespace DirectX;

	//! @brief コンストラクタ
	DebugRenderSystem::DebugRenderSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{

	}

	//! @brief 初期化
	void DebugRenderSystem::Init()
	{
#if defined(DEBUG) || defined(_DEBUG)
		auto& device = engine_->GetGraphicsDevice();

		cb_per_frame_ = device.CreateConstantBuffer({
			sizeof(CBPerFrame),
			nullptr
			});

		cb_per_object_ = device.CreateConstantBuffer({
			sizeof(CBPerObject),
			nullptr
			});

		// ハンドルの取得
		cube_mesh_.handle = engine_->GetMeshRegistry().GetHandleByName("Cube");
		sphere_mesh_.handle = engine_->GetMeshRegistry().GetHandleByName("Sphere");


		debug::DebugUI::ResistDebugFunction([this]()
			{
				if (ImGui::Begin("Debug Render")) {
					ImGui::Checkbox("Show All Colliders", &show_all_colliders_);
				}
				ImGui::End();
			}
		);
#endif
	}


	//! @brief 線分描画
	void DebugRenderSystem::DrawLine(XMFLOAT3 _start, XMFLOAT3 _end, DirectX::XMFLOAT4 _color)
	{

	}

	//! @brief 立方体描画
	void DebugRenderSystem::DrawCube(const Transform& _transform, DirectX::XMFLOAT4 _color)
	{
		DebugCommand cmd;
		cmd.mesh = cube_mesh_;
		const auto S = XMMatrixScaling(_transform.scale.x, _transform.scale.y, _transform.scale.z);
		const auto R = XMMatrixRotationQuaternion(XMLoadFloat4(&_transform.rotationQuat));
		const auto T = XMMatrixTranslation(_transform.position.x, _transform.position.y, _transform.position.z);
		cmd.world = S * R * T;
		cmd.color = _color;
		commands_.emplace_back(std::move(cmd));
	}


	// -------------------- 球描画 -------------------- // 
	//! @brief Transform版
	void DebugRenderSystem::DrawSphere(const Transform& _transform, DirectX::XMFLOAT4 _color)
	{
		DebugCommand cmd;
		cmd.mesh = sphere_mesh_;
		const auto S = XMMatrixScaling(_transform.scale.x, _transform.scale.y, _transform.scale.z);
		const auto R = XMMatrixRotationQuaternion(XMLoadFloat4(&_transform.rotationQuat));
		const auto T = XMMatrixTranslation(_transform.position.x, _transform.position.y, _transform.position.z);
		cmd.world = S * R * T;
		cmd.color = _color;
		commands_.emplace_back(std::move(cmd));
	}
	//! @brief 中心座標＋半径版
	void DebugRenderSystem::DrawSphere(const DirectX::XMFLOAT3& _center, float _radius, DirectX::XMFLOAT4 _color)
	{
		DebugCommand cmd;
		cmd.mesh = sphere_mesh_;
		cmd.world =
			XMMatrixScaling(_radius * 2.0f, _radius * 2.0f, _radius * 2.0f) *
			XMMatrixTranslation(_center.x, _center.y, _center.z);
		cmd.color = _color;
		commands_.emplace_back(std::move(cmd));
	}
	//! @brief WorldSphere版
	void DebugRenderSystem::DrawSphere(const collision::WorldSphere& _sphere, DirectX::XMFLOAT4 _color)
	{
		DebugCommand cmd;
		cmd.mesh = sphere_mesh_;
		cmd.world =
			XMMatrixScaling(_sphere.radius * 2.0f, _sphere.radius * 2.0f, _sphere.radius * 2.0f) *
			XMMatrixTranslation(_sphere.center.x, _sphere.center.y, _sphere.center.z);
		cmd.color = _color;
		commands_.emplace_back(std::move(cmd));
	}
	// ! @brief WorldSphereワイヤーフレーム版
	void DebugRenderSystem::DrawSphereWireframe(const collision::WorldSphere& _sphere, DirectX::XMFLOAT4 _color)
	{
		DebugCommand cmd;
		cmd.mesh = sphere_mesh_;
		cmd.world =
			XMMatrixScaling(_sphere.radius * 2.0f, _sphere.radius * 2.0f, _sphere.radius * 2.0f) *
			XMMatrixTranslation(_sphere.center.x, _sphere.center.y, _sphere.center.z);
		cmd.color = _color;
		cmd.wireframe = true;
		commands_.emplace_back(std::move(cmd));
	}

	//! @brief ポイント描画
	void DebugRenderSystem::DrawPoint(const DirectX::XMFLOAT3& _position, DirectX::XMFLOAT4 _color, float _size)
	{
		DebugCommand cmd;
		cmd.mesh = sphere_mesh_;
		cmd.world =
			XMMatrixScaling(_size, _size, _size) *
			XMMatrixTranslation(_position.x, _position.y, _position.z);
		cmd.color = _color;
		commands_.emplace_back(std::move(cmd));
	}


	//! @brief OBB描画
	void DebugRenderSystem::DrawOBB(const collision::WorldOBB& _obb, DirectX::XMFLOAT4 _color)
	{
		DebugCommand cmd;
		cmd.mesh = cube_mesh_;

		// OBBの軸から回転行列を構築
		XMMATRIX rotationMatrix = XMMATRIX(
			XMLoadFloat3(&_obb.axis[0]),  // 行0: X軸
			XMLoadFloat3(&_obb.axis[1]),  // 行1: Y軸
			XMLoadFloat3(&_obb.axis[2]),  // 行2: Z軸
			XMVectorSet(0, 0, 0, 1)       // 行3: 平行移動なし
		);
		rotationMatrix = XMMatrixTranspose(rotationMatrix);

		// Cubeメッシュは1x1x1なので、halfの2倍でスケーリング
		const auto S = XMMatrixScaling(_obb.half.x * 2.0f, _obb.half.y * 2.0f, _obb.half.z * 2.0f);
		const auto T = XMMatrixTranslation(_obb.center.x, _obb.center.y, _obb.center.z);

		cmd.world = S * rotationMatrix * T;
		cmd.color = _color;
		commands_.emplace_back(std::move(cmd));
	}
	//! @brief OBBワイヤーフレーム描画
	void DebugRenderSystem::DrawOBBWireframe(const collision::WorldOBB& _obb, DirectX::XMFLOAT4 _color)
	{
		DebugCommand cmd;
		cmd.mesh = cube_mesh_;
		XMMATRIX rot = XMMATRIX(
			XMLoadFloat3(&_obb.axis[0]),
			XMLoadFloat3(&_obb.axis[1]),
			XMLoadFloat3(&_obb.axis[2]),
			XMVectorSet(0, 0, 0, 1));
		rot = XMMatrixTranspose(rot);
		const auto S = XMMatrixScaling(_obb.half.x * 2.0f, _obb.half.y * 2.0f, _obb.half.z * 2.0f);
		const auto T = XMMatrixTranslation(_obb.center.x, _obb.center.y, _obb.center.z);
		cmd.world = S * rot * T;
		cmd.color = _color;
		cmd.wireframe = true;
		commands_.emplace_back(std::move(cmd));
	}

	//! @brief すべてのコライダー描画
	void DebugRenderSystem::DrawAllColliders(float _alpha)
	{
#if defined(DEBUG) || defined(_DEBUG)
		// Collider を持つ全 Entity を取得
		auto entities = ecs_.GetEntitiesWithComponent<Collider>();

		for (auto& e : entities) {
			auto& col = ecs_.GetComponent<Collider>(e);
			DirectX::XMFLOAT4 color = { 0.0f, 1.0f, 1.0f, _alpha };  // シアン

			switch (col.type) {
			case collision::ShapeType::Box:
				DrawOBBWireframe(col.worldOBB, color);
				break;
			case collision::ShapeType::Sphere:
				DrawSphereWireframe(col.worldSphere, color);
				break;
			default:
				break;
			}
		}
#endif
	}

	//! @brief 更新
	void DebugRenderSystem::Update(float _dt)
	{
#if defined(DEBUG) || defined(_DEBUG)
		if (show_all_colliders_) {
			DrawAllColliders(0.5f);
		}


		// コマンドがなければスキップ
		if (commands_.empty()) { return; }

		auto& dc = engine_->GetDeferredContext();
		ID3D11PixelShader* prevPS = nullptr;


		// カメラ取得
		Entity cameraEnt;
		auto camEntities = ecs_.GetEntitiesWithComponent<Camera>();
		if (camEntities.empty()) {
			GameLogWarning("CameraComponentを持つEntityが存在しないため、描画をスキップ");
			return;
		}
		// メインカメラを探す
		for (auto& e : camEntities) {
			auto cameraComp = ecs_.GetComponent<Camera>(e);
			if (cameraComp.isActive && cameraComp.isMain) {
				cameraEnt = e;
				break;
			}
		}
		if (!cameraEnt.IsInitialized()) {
			GameLogWarning("[DebugRenderSystem] アクティブなメインカメラが存在しない");
			return;
		}
		auto& cam = ecs_.GetComponent<Camera>(cameraEnt);

		// 定数バッファ更新
		// フレーム単位の定数バッファ更新
		CBPerFrame cbPerFrameData{};
		cbPerFrameData.view = cam.view;
		cbPerFrameData.proj = cam.proj;

		cb_per_frame_->Update(dc, &cbPerFrameData, sizeof(cbPerFrameData));
		dc.VSSetConstantBuffer(0, *cb_per_frame_);

		// 描画
		for (auto& cmd : commands_) {
			// メッシュの取得
			auto mesh = engine_->GetMeshRegistry().Get(cmd.mesh.handle);
			// オブジェクト単位のCB更新
			CBPerObject cbPerObjectData{};
			cbPerObjectData.world = cmd.world;
			cbPerObjectData.color = cmd.color;
			cb_per_object_->Update(dc, &cbPerObjectData, sizeof(cbPerObjectData));
			dc.VSSetConstantBuffer(1, *cb_per_object_);

			auto psoKey = dx3d::BuildPipelineKey(
				dx3d::VertexShaderKind::Default,
				dx3d::PixelShaderKind::Color,
				dx3d::BlendMode::Alpha,
				dx3d::DepthMode::ReadOnly,
				cmd.wireframe ? dx3d::RasterMode::Wireframe : dx3d::RasterMode::SolidBack
			);

			engine_->Render(*mesh->vb, *mesh->ib, psoKey);
		}
		commands_.clear();



#endif // DEBUG || _DEBUG
	}
}

