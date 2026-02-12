#pragma once
/**
 * @file SpriteRenderSystem.h
 * @brief スプライト描画システム（UI想定）
 * @author Arima Keita
 * @date 2025-09-01
 */

 // ---------- インクルード ---------- //
#include <wrl/client.h>
#include <DX3D/Core/Core.h>
#include <Game/ECS/ISystem.h>

#include <DX3D/Graphics/PipelineCache.h>
#include <DX3D/Graphics/Buffers/InstanceData.h>
#include <DX3D/Graphics/Textures/TextureHandle.h>
// ---------- 前方宣言 ---------- //
namespace dx3d {
	class GraphicsEngine;
	class Texture;
	class Mesh;
}

namespace ecs {

	/**
	 * @brief スプライト描画システム
	 * 
	 * @detail
	 * - MeshRenderer.handle は Quad を指す想定（MeshRegistryを流用してVB/IBを取得する）
	 * - UI想定なので正射影(Ortho)で描画する
	 */
	class SpriteRenderSystem : public ISystem {
	public:
		explicit SpriteRenderSystem(const SystemDesc& _desc);
		void Init() override;
		void Update(float _dt) override;

	private:
		//! @brief バッチ収集
		void CollectBatches();
		//! @brief バッチ更新
		void UpdateBatches();
		//! @brief 描画
		void RenderSpritePass();
		//! @brief インスタンスバッファの作成またはリサイズ
		void CreateOrResizeInstanceBuffer(size_t _requiredInstanceCapacity);

		struct InstanceBatchSprite {
			dx3d::VertexBufferPtr vb{};
			dx3d::IndexBufferPtr ib{};
			uint32_t indexCount{};
			std::vector<dx3d::InstanceDataSprite> instances{};
			size_t instanceOffset = 0;

			dx3d::PipelineKey pipelineKey{};
			dx3d::TextureHandle textureHandle{};
			int layer = 0; // 小さいほど手前に描画される。
		};

	private:
		dx3d::PipelineKey pso_key_{};
		dx3d::Mesh* quad_mesh_{};
		
		// バッチ
		std::vector<InstanceBatchSprite> batches_{};
		// インスタンスバッファ
		std::shared_ptr<dx3d::VertexBuffer> instance_buffer_{};
		size_t instance_buffer_capacity_{};

		// 定数バッファ
		dx3d::ConstantBufferPtr cb_per_frame_{};
		// UI用サンプラ
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_linear_clamp_{};
		





	};

}
