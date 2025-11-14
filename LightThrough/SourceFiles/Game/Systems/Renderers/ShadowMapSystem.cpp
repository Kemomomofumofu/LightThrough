/**
 * @file ShadowMapSystem.cpp
 */

 // ---------- インクルード ---------- // 
#include <DirectXMath.h>
#include <Game/Systems/Renderers/ShadowMapSystem.h>

#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/Meshes/MeshRegistry.h>
#include <DX3D/Graphics/Meshes/Mesh.h>
#include <DX3D/Graphics/Buffers/ConstantBuffer.h>

#include <Game/ECS/Coordinator.h>

#include <Game/Components/Transform.h>
#include <Game/Components/Light.h>
#include <Game/Components/MeshRenderer.h>

namespace {

	// RenderSystem と同等レイアウト（slot1, row_major 前提）
	struct CBPerObject {
		DirectX::XMMATRIX world;	// ワールド行列（非転置）
		DirectX::XMFLOAT4 color;	// 未使用（レイアウト維持用）
	};
}

namespace ecs {

	void ShadowMapSystem::Init()
	{
		
		// 必須コンポーネント（ライト＋トランスフォーム）
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<LightCommon>());
		ecs_.SetSystemSignature<ShadowMapSystem>(signature);

		// シャドウパス用定数バッファ作成
		auto& device = engine_->GetGraphicsDevice();

		cb_per_object_ = device.CreateConstantBuffer({
			sizeof(CBPerObject),
			nullptr
			});

		cb_light_matrix_ = device.CreateConstantBuffer({
			sizeof(CBLightMatrix),
			nullptr
			});
	}

	void ShadowMapSystem::Update(float _dt)
	{
		// ライトが無ければ何もしない
		if (entities_.empty()) { return; }

		// 必要なライト分のシャドウマップを確保
		for (auto light : entities_) {
			auto& common = ecs_.GetComponent<LightCommon>(light);
			if (!common.enabled) { continue; }
			(void)GetOrCreateShadowMap(light);
		}

		// シャドウパス実行
		DrawShadowPasses();
	}

	//! @brief エンティティ破棄イベント
	void ShadowMapSystem::OnEntityDestroyed(Entity _e)
	{
		// シャドウマップ削除
		auto it = entity_shadow_map_.find(_e);
		if (it != entity_shadow_map_.end()) {
			entity_shadow_map_.erase(it);
		}
	}

	ID3D11ShaderResourceView* ShadowMapSystem::GetShadowSRV(Entity _light) const
	{
		auto it = entity_shadow_map_.find(_light);
		if (it == entity_shadow_map_.end()) { return nullptr; }
		return it->second.srv.Get();
	}

	/**
	 * @brief ライト用シャドウマップ取得、無ければ作成
	 * @param _light ライトEntity
	 * @return シャドウマップリソースの参照
	 */
	ShadowMap& ShadowMapSystem::GetOrCreateShadowMap(Entity _light)
	{
		auto it = entity_shadow_map_.find(_light);
		if (it != entity_shadow_map_.end()) { return it->second; }

		// シャドウマップ作成
		ShadowMap shadowMap{};
		auto& device = engine_->GetGraphicsDevice();
		// シャドウマップ用テクスチャ作成
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = 1024;
		texDesc.Height = 1024;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		device.CreateTexture2D(&texDesc, nullptr, &shadowMap.depthTex);

		// DSV 作成
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		device.CreateDepthStencilView(shadowMap.depthTex.Get(), &dsvDesc, &shadowMap.dsv);

		// SRV 作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		device.CreateShaderResourceView(shadowMap.depthTex.Get(), &srvDesc, &shadowMap.srv);

		// マップに登録
		auto [insertIt, _] = entity_shadow_map_.emplace(_light, std::move(shadowMap));
		return insertIt->second;
	}

	/**
	 * @brief シャドウパスの描画
	 *
	 * 各ライト毎にライト行列を更新し、深度オンリーで全メッシュを描画する
	 */
	void ShadowMapSystem::DrawShadowPasses()
	{
		using namespace DirectX;

		// SRV が PS にバインド済みの可能性があるのでアンバインド
		{
			auto context = engine_->GetDeviceContext().GetDeviceContext();
			ID3D11ShaderResourceView* nullSRV[16] = {};
			context->PSSetShaderResources(0, 16, nullSRV);
		}

		// ライト毎に描画
		for (auto& [lightEntity, sm] : entity_shadow_map_) {
			if (!sm.dsv) { continue; }

			// ライトが存在・有効かチェック
			if (!ecs_.HasComponent<LightCommon>(lightEntity) || !ecs_.HasComponent<Transform>(lightEntity)) { continue; }
			auto& common = ecs_.GetComponent<LightCommon>(lightEntity);
			if (!common.enabled) { continue; }
			auto& ltf = ecs_.GetComponent<Transform>(lightEntity);

			// ライト行列計算（row_major想定なので非転置）
			const XMFLOAT3 lpos{ ltf.position.x, ltf.position.y, ltf.position.z };
			const XMFLOAT3 ldir = ltf.GetForward(); // 前方ベクトル

			auto ComputeLightVP = [](XMFLOAT3 _pos, XMFLOAT3 _dir, float _fovYDeg, float _aspect, float _nearZ, float _farZ)
				{
					XMVECTOR eye = XMLoadFloat3(&_pos);
					XMVECTOR d = XMVector3Normalize(XMLoadFloat3(&_dir));
					XMVECTOR up = XMVectorSet(0, 1, 0, 0);

					XMMATRIX V = XMMatrixLookToLH(eye, d, up);
					XMMATRIX P = XMMatrixPerspectiveFovLH(XMConvertToRadians(_fovYDeg), _aspect, _nearZ, _farZ);
					return V * P;
				};

			// スポットライトは外側コサインから概ねのFOVを推定、無ければデフォルト45度
			float fovYDeg = 45.0f;
			if (ecs_.HasComponent<SpotLight>(lightEntity)) {
				auto& spot = ecs_.GetComponent<SpotLight>(lightEntity);
				// 注意: outerCos はコサイン値、FOV ? 2 * acos(outerCos)
				fovYDeg = XMConvertToDegrees(2.0f * acosf((std::max)(-1.0f, (std::min)(1.0f, spot.outerCos))));
			}

			XMMATRIX lightVP = ComputeLightVP(lpos, ldir, fovYDeg, 1.0f, 0.1f, 100.0f);

			// ライト行列の更新（VS slot2）
			CBLightMatrix cb{};
			cb.lightViewProj = lightVP;
			cb_light_matrix_->Update(engine_->GetDeviceContext(), &cb, sizeof(cb));
			engine_->GetDeviceContext().VSSetConstantBuffer(2, *cb_light_matrix_);

			// 描画ターゲット設定
			//engine_->SetDepthOnlyTarget(sm.dsv.Get());
			//engine_->ClearDepth(sm.dsv.Get(), 1.0f, 0);
			//engine_->SetViewport(1024, 1024);

			// シャドウマップ用パイプライン
			dx3d::PipelineKey key{ dx3d::VertexShaderKind::ShadowMap, dx3d::PixelShaderKind::ShadowMap };
			//engine_->SetPipeline(key);

			// バイアス設定（シャドウアクネ対策）
			//engine_->SetRasterizerBias(100, 1.5f, 0.0f);

			auto& context = engine_->GetDeviceContext();

			// 全メッシュを描画
			for (auto e : ecs_.GetEntitiesWithComponent<MeshRenderer>()) {
				auto& tf = ecs_.GetComponent<Transform>(e);
				auto& mr = ecs_.GetComponent<MeshRenderer>(e);

				auto& reg = engine_->GetMeshRegistry();
				auto meshData = reg.Get(mr.handle);
				if (!meshData || !meshData->vb || !meshData->ib) { continue; }

				// PerObject 更新（VS slot1）
				CBPerObject obj{};
				obj.world = DirectX::XMLoadFloat4x4(&tf.world);
				obj.color = { 1,1,1,1 }; // 未使用
				cb_per_object_->Update(context, &obj, sizeof(obj));
				context.VSSetConstantBuffer(1, *cb_per_object_);

				// 描画
				engine_->Render(*meshData->vb, *meshData->ib);
			}
		}
	}

}