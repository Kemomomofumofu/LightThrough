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
#include <Game/Components/Mesh.h>
#include <DX3D/Graphics/Buffers/ConstantBuffer.h>


namespace dx3d {
	class GraphicsEngine;
}

namespace ecs {
	// ---------- 名前空間 ---------- // 
	using namespace DirectX;

	/**
	 * @brief デバッグ描画システム
	 *
	 * 当たり判定などの描画に使う
	 * DrawHOGE()で呼ばれたものをCommandとして保持して描画時に書き出す
	 */
	class DebugRenderSystem : public ISystem {
	public:
		

		DebugRenderSystem(const dx3d::SystemDesc& _desc);
		void Init(const ecs::Coordinator& _ecs);
		void SetGraphicsEngine(dx3d::GraphicsEngine& _engine) { engine_ = &_engine; }

		void DrawLine(XMFLOAT3 _start, XMFLOAT3 _end, XMFLOAT4 _color);
		void DrawCube(const Transform& _transform, XMFLOAT4 _color);
		void DrawSphere(const Transform& _transform, XMFLOAT4 _color);

		void Update(float _dt, Coordinator& _ecs) override;


	private:
		// Command構造体
		struct DebugCommand {
			Mesh mesh;
			XMMATRIX world;
			XMFLOAT4 color;
		};

		dx3d::GraphicsEngine* engine_{};
		std::vector<DebugCommand> commands_{};
		dx3d::ConstantBufferPtr cb_per_frame_{};
		dx3d::ConstantBufferPtr cb_per_object_{};

		// Meshのキャッシュ
		Mesh cube_mesh_{};
		Mesh sphere_mesh_{};
		Mesh line_mesh_{};
	};
}
