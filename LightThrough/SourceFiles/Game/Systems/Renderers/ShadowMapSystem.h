#pragma once
/**
 * @file ShadowMapSystem.h
 * @brief シャドウマップの管理
 */

 // ---------- インクルード ---------- // 
#include <wrl/client.h>
#include <unordered_map>
#include <DX3D/Core/Core.h>
#include <Game/ECS/ISystem.h>

namespace ecs {

	// シャドウマップ用リソース
	struct ShadowMap
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTex{};
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv{};
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv{};
	};

	class ShadowMapSystem : public ISystem
	{
	public:
		explicit ShadowMapSystem(const SystemDesc& _desc)
			: ISystem(_desc) {
		}

		void Init();
		void SetGraphicsEngine(dx3d::GraphicsEngine& _engine) { engine_ = &_engine; }

		void Update(float _dt) override;

		void OnEntityDestroyed(Entity _e) override;

		// メインパスでピクセルシェーダへバインドするための SRV 取得
		ID3D11ShaderResourceView* GetShadowSRV(Entity _light) const;

	private:
		//! @brief ライト用シャドウマップ取得、無ければ作成
		ShadowMap& GetOrCreateShadowMap(Entity _light);
		//! @brief シャドウパスの描画
		void DrawShadowPasses();

	private:
		dx3d::GraphicsEngine* engine_{};

		std::unordered_map<Entity, ShadowMap> entity_shadow_map_{}; // ライト毎のシャドウマップ

		// シャドウパス用定数バッファ
		dx3d::ConstantBufferPtr cb_per_object_{};
		dx3d::ConstantBufferPtr cb_light_matrix_{};
	};

}