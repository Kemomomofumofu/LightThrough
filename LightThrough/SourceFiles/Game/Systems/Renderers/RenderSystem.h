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
#include <Game/ECS/ISystem.h>

// ---------- 前方宣言 ---------- //
namespace dx3d {
	class GraphicsEngine;
}

namespace ecs {
	/**
	 * @brief 描画システム
	 *
	 * @detail 必須；<Transform>, <Mesh>
	 */
	class RenderSystem : public ISystem {
	public:

		//! @brief インスタンス描画用バッチ構造体
		// mainPass
		struct InstanceBatchMain {
			dx3d::VertexBufferPtr vb{};
			dx3d::IndexBufferPtr ib{};
			uint32_t indexCount{};
			std::vector<dx3d::InstanceDataMain> instances{};
			size_t instanceOffset = 0;
		};
		// shadowPass
		struct InstanceBatchShadow {
			dx3d::VertexBufferPtr vb{};
			dx3d::IndexBufferPtr ib{};
			uint32_t indexCount{};
			std::vector<dx3d::InstanceDataShadow> instances{};
			size_t instanceOffset = 0;
		};



		//! @brief コンストラクタ
		explicit RenderSystem(const SystemDesc& _desc);
		//! @brief 初期化
		void Init();
		void SetGraphicsEngine(dx3d::GraphicsEngine& _engine) { engine_ = &_engine; }
		//! @brief 更新
		void Update(float _dt) override;
		//! @brief 破棄イベント
		void OnEntityDestroyed(Entity _entity) override;
	private:
		//! @brief バッチクリア
		void ClearBatches();
		//! @brief バッチ収集
		void CollectBatches();
		//! @brief バッチ更新
		void UpdateBatches();
		//! @brief 描画
		void RenderMainPass(struct CBLight& _lightData);
		//! @brief インスタンスバッファの作成またはリサイズ
		void CreateOrResizeInstanceBuffer(size_t _requiredInstanceCapacity);



	private:
		dx3d::GraphicsEngine* engine_{};

		dx3d::ConstantBufferPtr cb_per_frame_{};
		dx3d::ConstantBufferPtr cb_per_object_{};	// [ToDo] 単体描画用/マテリアル毎とか？？？
		dx3d::ConstantBufferPtr cb_lighting_{};
		dx3d::ConstantBufferPtr cb_light_matrix_{};

		std::shared_ptr<dx3d::VertexBuffer> instance_buffer_{};
		size_t instance_buffer_capacity_{};

		std::vector<InstanceBatchMain> main_batches_{}; // メインパスのバッチ
		std::vector<InstanceBatchShadow> shadow_batches_{}; // シャドウパスのバッチ

	private:
		// memo: シンプルな落ち影の実装
		// todo: リソース管理は別クラスにて行いたい
		// シャドウマップ用リソース
		void CreateShadowResources(uint32_t _size);
		void RenderShadowPass(const DirectX::XMMATRIX& _lightViewProj);
		const uint32_t SHADOW_MAP_SIZE = 1024;


		// シャドウマップ用リソース
		Microsoft::WRL::ComPtr<ID3D11Texture2D> shadow_depth_tex_{};
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadow_dsv_{};
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadow_srv_{};
		Microsoft::WRL::ComPtr<ID3D11SamplerState> shadow_sampler_{};

	};

}