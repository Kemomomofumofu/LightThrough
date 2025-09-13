#pragma once
/**
 * @file DebugDrawSystem.h
 * @brief デバッグ用の描画システム
 * @author Arima Keita
 * @date 2025-09-12
 */

 /*---------- インクルード ----------*/
#include <vector>
#include <DirectXMath.h>
#include <DX3D/Game/ECS/ISystem.h>
#include <Game/Components/Mesh.h>
#include <DX3D/Graphics/ConstantBuffer.h>


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
	class DebugDrawSystem : public ISystem {
	public:
		void Init(ecs::Coordinator& _ecs);
		void SetGraphicsEngine(dx3d::GraphicsEngine& _engine) { engine_ = &_engine; }

		void DrawLine(XMFLOAT3 _start, XMFLOAT3 _end, XMFLOAT4 _color);
		void DrawCube(XMFLOAT3 _pos, XMFLOAT3 _size, XMFLOAT4 _color);
		void DrawSphere(XMFLOAT3 _pos, float _radius, XMFLOAT4 _color);

		void Update(float _dt, Coordinator& _ecs) override;


	private:
		// Command構造体
		struct DebugCommand {
			Mesh mesh;
			XMMATRIX world;
			XMFLOAT4 color;
		};

		dx3d::GraphicsEngine* engine_{};
		std::vector<DebugCommand> commands_;
		dx3d::ConstantBufferPtr cb_per_frame_{};
		dx3d::ConstantBufferPtr cb_per_object_{};

		// Meshのキャッシュ
		Mesh cube_mesh_{};
		Mesh sphere_mesh_{};
		Mesh line_mesh_{};
	};
}
