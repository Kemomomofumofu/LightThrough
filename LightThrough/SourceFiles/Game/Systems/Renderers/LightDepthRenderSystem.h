#pragma once
/**
 * @file LightDepthRenderSystem.h
 * @brief 描画システム
 * @author Arima Keita
 * @date 2025-09-01
 */

 // ---------- インクルード ---------- // 
#include <wrl/client.h>
#include <unordered_map>
#include <DX3D/Core/Core.h>
#include <DX3D/Graphics/Buffers/InstanceData.h>
#include <Game/ECS/ISystem.h>

// ---------- 前方宣言 ---------- //
namespace dx3d {
	class GraphicsEngine;
}

namespace ecs {
	//! @brief インスタンス描画用バッチ構造体
	// shadowPass
	struct InstanceBatchShadow {
		dx3d::VertexBufferPtr vb{};
		dx3d::IndexBufferPtr ib{};
		uint32_t indexCount{};
		std::vector<dx3d::InstanceDataShadow> instances{};
		size_t instanceOffset = 0;
	};

	//! @brief ライトごとのシャドウ情報エントリ
	struct ShadowLightEntry {
		ecs::Entity light;
		DirectX::XMFLOAT4X4 lightViewProj;
		int32_t sliceIndex = -1;
	};

	/**
	 * @brief 描画システム
	 *
	 * @detail 必須；<Transform>, <Mesh>
	 */
	class LightDepthRenderSystem : public ISystem {
	public:
		explicit LightDepthRenderSystem(const SystemDesc& _desc);
		//! @brief 初期化
		void Init() override;
		//! @brief 更新
		void Update(float _dt) override;
		/**
		 * @brief エンティティ破棄時処理
		 * @param _e 破棄されたエンティティ
		 */
		void OnEntityDestroyed(Entity _e) override;

		/**
		 * @brief シャドウマップSRV取得
		 * @return SRVポインタ
		 */
		ID3D11ShaderResourceView* GetShadowMapSRVs() const { return shadow_srvs_.Get(); }

		/**
		 * @brief シャドウマップ用サンプラーステート取得
		 * @return サンプラーステートポインタ
		 */
		ID3D11SamplerState* GetShadowSampler() const { return shadow_sampler_.Get(); }

		uint32_t GetShadowMapWidth() const { return SHADOW_MAP_WIDTH; }
		uint32_t GetShadowMapHeight() const { return SHADOW_MAP_HEIGHT; }

		//! @brief ライトエンティティのシャドウ情報取得
		std::vector<ShadowLightEntry> GetShadowLights() const { return shadow_lights_; }
	private:
		//! @brief バッチ収集
		void CollectBatches();
		//! @brief バッチ更新
		void UpdateBatches();
		//! @brief シャドウマップ描画パス
		void RenderShadowPass(ShadowLightEntry _entry, ID3D11DepthStencilView* _dsv);
		//! @brief インスタンスバッファの作成またはリサイズ
		void CreateOrResizeInstanceBufferShadow(size_t _requiredInstanceCapacity);
		// シャドウマップ用リソースの作成
		void CreateShadowResources(uint32_t _texHeight, uint32_t _texWidth, uint32_t _arraySize);


	private:
		std::weak_ptr<class DebugRenderSystem> debug_render_system_{};


		static const uint32_t MAX_SHADOW_LIGHTS = 16;
		const uint32_t SHADOW_MAP_WIDTH = 2048;
		const uint32_t SHADOW_MAP_HEIGHT = 2048;

		std::vector<InstanceBatchShadow> shadow_batches_{}; // シャドウパスのバッチ
		std::shared_ptr<dx3d::VertexBuffer> instance_buffer_shadow_{};
		size_t instance_buffer_capacity_{};
		dx3d::ConstantBufferPtr cb_light_matrix_{};	// 定数バッファ

		// シャドウマップ用リソース
		Microsoft::WRL::ComPtr<ID3D11Texture2D> shadow_depth_tex_{};
		std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> shadow_dsvs_{};
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadow_srvs_{};
		//std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> shadow_srvs_{};
		Microsoft::WRL::ComPtr<ID3D11SamplerState> shadow_sampler_{};

		// ライトごとのシャドウ情報
		std::vector<ShadowLightEntry> shadow_lights_;

	};

}