#pragma once
/**
 * @file DebugRenderSystem.h
 * @brief デバッグ用の描画システム
 * @author Arima Keita
 * @date 2025-09-12
 */

 /*---------- インクルード ----------*/
#include <vector>
#include <DirectXMath.h>
#include <Game/ECS/ISystem.h>
#include <Game/Components/MeshRenderer.h>
#include <DX3D/Graphics/Buffers/ConstantBuffer.h>

#include <Game/Collisions/CollisionUtils.h>


namespace dx3d {
	class GraphicsEngine;
}


namespace ecs {
	// ---------- 名前空間 ---------- // 
	struct Transform;

	/**
	 * @brief デバッグ描画システム
	 *
	 * 当たり判定などの描画に使う
	 * DrawHOGE()で呼ばれたものをCommandとして保持して描画時に書き出す
	 */
	class DebugRenderSystem : public ISystem {
	public:
		DebugRenderSystem(const SystemDesc& _desc);
		void Init() override;
		void SetGraphicsEngine(dx3d::GraphicsEngine& _engine) { engine_ = &_engine; }

		void DrawLine(DirectX::XMFLOAT3 _start, DirectX::XMFLOAT3 _end, DirectX::XMFLOAT4 _color = { 1.0f, 1.0f, 1.0f, 1.0f });
		void DrawCube(const Transform& _transform, DirectX::XMFLOAT4 _color = { 1.0f, 1.0f, 1.0f, 1.0f });

		void DrawSphere(const Transform& _transform, DirectX::XMFLOAT4 _color = { 1.0f, 1.0f, 1.0f, 1.0f });
		void DrawSphere(const DirectX::XMFLOAT3& _center, float _radius, DirectX::XMFLOAT4 _color = { 1.0f, 1.0f, 1.0f, 1.0f });
		void DrawSphere(const collision::WorldSphere& _sphere, DirectX::XMFLOAT4 _color = {1.0f, 1.0f, 1.0f, 1.0f });
		void DrawSphereWireframe(const collision::WorldSphere& _sphere, DirectX::XMFLOAT4 _color = { 1.0f, 1.0f, 1.0f, 1.0f });

		/**
		 * @brief ポイント描画
		 * @param _position 位置
		 * @param _color 色
		 * @param _size サイズ
		 */
		void DrawPoint(const DirectX::XMFLOAT3& _position, DirectX::XMFLOAT4 _color = { 1.0f, 1.0f, 1.0f, 1.0f }, float _size = 0.1f);

		/**
		 * @brief OBB描画
		 * @param _obb OBB情報
		 * @param _color 色
		 */
		void DrawOBB(const collision::WorldOBB& _obb, DirectX::XMFLOAT4 _color = { 1.0f, 1.0f, 1.0f, 1.0f });
		void DrawOBBWireframe(const collision::WorldOBB& _obb, DirectX::XMFLOAT4 _color = { 1.0f, 1.0f, 1.0f, 1.0f });


		void DrawAllColliders(float _alpha = 0.5f);


		/**
		 * @brief 更新
		 */
		void Update(float _dt) override;


	private:
		// Command構造体
		struct DebugCommand {
			MeshRenderer mesh;
			DirectX::XMMATRIX world;
			DirectX::XMFLOAT4 color;
			bool wireframe = false;
		};

		dx3d::GraphicsEngine* engine_{};
		std::vector<DebugCommand> commands_{};
		dx3d::ConstantBufferPtr cb_per_frame_{};
		dx3d::ConstantBufferPtr cb_per_object_{};

		// Meshのキャッシュ
		MeshRenderer cube_mesh_{};
		MeshRenderer sphere_mesh_{};
		MeshRenderer line_mesh_{};

		bool show_all_colliders_ = false;
	};
}
