#pragma once
/**
 * @file RenderSystem.h
 * @brief 描画システム
 * @author Arima Keita
 * @date 2025-09-01
 */

 // ---------- インクルード ---------- // 
#include <wrl/client.h>
#include <DX3D/Core/Core.h>
#include <DX3D/Graphics/Buffers/InstanceData.h>
#include <DX3D/Graphics/PipelineCache.h>
#include <Game/ECS/ISystem.h>

// ---------- 前方宣言 ---------- //
namespace dx3d {
	class GraphicsEngine;
}

namespace ecs {
	struct CBLight;

	/**
	 * @brief 描画システム
	 *
	 * @detail 必須；<Transform>, <Mesh>
	 */
	class RenderSystem : public ISystem {
	public:
		//! @brief コンストラクタ
		explicit RenderSystem(const SystemDesc& _desc);
		//! @brief 初期化
		void Init() override;
		//! @brief 更新
		void Update(float _dt) override;

		ID3D11ShaderResourceView* GetDepthSRV() const { return depth_srv_.Get(); }
		ID3D11ShaderResourceView* GetSceneColorSRV() const { return scene_color_srv_.Get(); }
	private:
		//! @brief バッチ収集
		void CollectBatches(const DirectX::XMFLOAT3& _camPos);
		//! @brief バッチ更新
		void UpdateBatches();
		//! @brief 描画
		void RenderMainPass(struct CBLight& _lightData);
		//! @brief インスタンスバッファの作成またはリサイズ
		void CreateOrResizeInstanceBufferMain(size_t _requiredInstanceCapacity);

		//! @brief インスタンス描画用バッチ構造体
		struct InstanceBatchMain {
			dx3d::VertexBufferPtr vb{};
			dx3d::IndexBufferPtr ib{};
			uint32_t indexCount{};
			std::vector<dx3d::InstanceDataMain> instances{};
			size_t instanceOffset = 0;
			dx3d::PipelineKey key{};
			float sortKey = 0.0f; // ソート用キー
		};
	private: 

		dx3d::GraphicsEngine& engine_;
		// バッチ
		std::vector<InstanceBatchMain> opaque_batches_{};
		std::vector<InstanceBatchMain> transparent_batches_{};
		// インスタンスバッファ
		std::shared_ptr<dx3d::VertexBuffer> instance_buffer_main_{};
		size_t instance_buffer_capacity_{};
		// 定数バッファ
		dx3d::ConstantBufferPtr cb_per_frame_{};
		dx3d::ConstantBufferPtr cb_per_object_{};	// [ToDo] 単体描画用/マテリアル毎とか？？？
		dx3d::ConstantBufferPtr cb_light_matrix_{};
		dx3d::ConstantBufferPtr cb_lighting_{};
		// 深度バッファ用SRV
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depth_srv_;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scene_color_srv_;
	};

}