#pragma once
/**
 * @file RenderSystem.h
 * @brief 描画システム
 * @author Arima Keita
 * @date 2025-09-01
 */

// ---------- インクルード ---------- // 
#include <DX3D/Core/Core.h>
#include <DX3D/Graphics/Buffers/ConstantBuffer.h>
#include <DX3D/Graphics/Buffers/InstanceData.h>
#include <Game/ECS/ISystem.h>

// ---------- 前方宣言 ---------- //
namespace dx3d {
	class GraphicsEngine;
}

namespace ecs {

	struct InstanceBatch {
		dx3d::VertexBufferPtr vb{};
		dx3d::IndexBufferPtr ib{};
		uint32_t indexCount{};
		std::vector<dx3d::InstanceData> instances{};
		size_t instanceOffset = 0;
	};

	/**
	 * @brief 描画システム
	 *
	 * 必須；<Transform>, <Mesh>
	 */
	class RenderSystem : public ISystem{
	public:
		RenderSystem(const SystemDesc& _desc);
		void Init();
		void SetGraphicsEngine(dx3d::GraphicsEngine& _engine) { engine_ = &_engine; }

		void Update(float _dt) override;
	private:
		void CollectBatches();
		void UploadAndDrawBatches();
		void CreateOrResizeInstanceBuffer(size_t _requiredInstanceCapacity);

	private:
		dx3d::GraphicsEngine* engine_{};
		dx3d::ConstantBufferPtr cb_per_frame_{};
		dx3d::ConstantBufferPtr cb_per_object_{};	// [ToDo] 単体描画用/マテリアル毎とか？？？

		std::shared_ptr<dx3d::VertexBuffer> instance_buffer_{};
		size_t instance_buffer_capacity_{};

		std::vector<InstanceBatch> batches_{};
	};

}