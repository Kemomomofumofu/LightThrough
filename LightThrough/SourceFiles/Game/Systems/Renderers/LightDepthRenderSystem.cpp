/**
 * @file LightDepthRenderSystem.cpp
 * @brief 描画システム
 * @author Arima Keita
 * @date 2025-09-01
 */

 // ---------- インクルード ---------- //
#include <DirectXMath.h>
#include <Game/Systems/Renderers/LightDepthRenderSystem.h>

#include <DX3D/Graphics/Buffers/ConstantBuffer.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/Meshes/MeshRegistry.h>
#include <DX3D/Graphics/Meshes/Mesh.h>
#include <DX3D/Graphics/GraphicslogUtils.h>
#include <DX3D/Graphics/PipelineKey.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Camera/Camera.h>
#include <Game/Components/Core/Transform.h>
#include <Game/Components/Render/MeshRenderer.h>
#include <Game/Components/Render/Light.h>

#include <Game/Systems/Renderers/DebugRenderSystem.h>

#include <Debug/Debug.h>

namespace {
	struct CBPerFrame {
		DirectX::XMMATRIX view;	// ビュー行列
		DirectX::XMMATRIX proj;	// プロジェクション行列
	};
	struct CBPerObject {
		DirectX::XMMATRIX world;	// ワールド行列
		DirectX::XMFLOAT4 color;	// 色
	};
}

namespace ecs {
	/**
	 * @brief コンストラクタ
	 */
	LightDepthRenderSystem::LightDepthRenderSystem(const SystemDesc& _desc)
		: ISystem(_desc)
		, engine_(_desc.graphicsEngine)
	{
		//light_view_proj_matrices_.resize(MAX_SHADOW_LIGHTS);
	}

	void LightDepthRenderSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<MeshRenderer>());
		// 2025-11-26 todo: shadowCasterComponentとか追加してもよいかも。
		ecs_.SetSystemSignature<LightDepthRenderSystem>(signature);

		auto& device = engine_.GetGraphicsDevice();
		// ConstantBuffer作成
		cb_light_matrix_ = device.CreateConstantBuffer({
			sizeof(CBLightMatrix),
			nullptr
			});

		{
			D3D11_SAMPLER_DESC sd{};
			sd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			sd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
			sd.BorderColor[0] = sd.BorderColor[1] = sd.BorderColor[2] = sd.BorderColor[3] = 1.0f;
			sd.MaxAnisotropy = 1;
			sd.MipLODBias = 0;
			sd.MinLOD = -FLT_MAX;
			sd.MaxLOD = FLT_MAX;
			engine_.GetGraphicsDevice().GetD3DDevice()->CreateSamplerState(&sd, &shadow_sampler_);
		}

		CreateShadowResources(SHADOW_MAP_HEIGHT, SHADOW_MAP_WIDTH, MAX_SHADOW_LIGHTS);

		debug_render_system_ = ecs_.GetSystem<DebugRenderSystem>();
	}


	//! @brief 更新処理
	void LightDepthRenderSystem::Update(float _dt)
	{
		// バッチ処理
		shadow_batches_.clear();	// バッチクリア
		CollectBatches();			// バッチ収集
		UpdateBatches();			// バッチ更新

		// todo: シーンリロード時に他Systemが、shadow_lights_のclearより先にshadow_lights_にアクセスしてクラッシュする問題がある
		shadow_lights_.clear();
		// 深度パス実行
		int32_t shadowIndex = 0;
		for (auto& e : ecs_.GetEntitiesWithComponents<LightCommon>()) {
			if (shadowIndex >= MAX_SHADOW_LIGHTS) { break; }

			auto common = ecs_.GetComponent<LightCommon>(e);
			if (!common->enabled) { continue; }
			auto tf = ecs_.GetComponent<Transform>(e);
			SpotLight* spot = nullptr;
			if (ecs_.HasComponent<SpotLight>(e)) {
				spot = ecs_.GetComponent<SpotLight>(e);
			}

			LightViewProj vp = BuildLightViewProj(tf, spot);

			// 情報の格納
			auto lightViewProj = vp.view * vp.proj;
			XMFLOAT4X4 lightVPFloat;
			XMStoreFloat4x4(&lightVPFloat, lightViewProj);
			shadow_lights_.push_back({ e, lightVPFloat, shadowIndex });
			//DebugLogInfo("[LightDepthRenderSystem] Dispatching ShadowTest CS for light {} sliceIndex={}", e.id_, shadowIndex);

			// 描画
			RenderShadowPass(shadow_lights_.back(), shadow_dsvs_[shadowIndex].Get());

			++shadowIndex;
		}
	}

	//! @brief エンティティ破棄時の処理
	void LightDepthRenderSystem::OnEntityDestroyed(Entity _e)
	{
		// ShadowLightEntryから削除
		auto it = std::remove_if(shadow_lights_.begin(), shadow_lights_.end(),
			[_e](const ShadowLightEntry& entry) {
				return entry.light == _e;
			});

		if (it != shadow_lights_.end()) {
			shadow_lights_.erase(it, shadow_lights_.end());
		}

	}


	/**
	 * @brief バッチ収集
	 *
	 * Meshの頂点バッファとインデックスバッファが同じものをまとめてバッチ化する
	 * インスタンスデータは各EntityのTransformからワールド行列を取得して格納する
	 */
	void LightDepthRenderSystem::CollectBatches()
	{
		struct Key {
			dx3d::VertexBuffer* vb{};
			dx3d::IndexBuffer* ib{};
			bool operator==(const Key& o) const noexcept { return vb == o.vb && ib == o.ib; }
		};

		struct KeyHash {
			size_t operator()(const Key& k) const noexcept {
				return std::hash<void*>()(k.vb) ^ (std::hash<void*>()(k.ib) << 1);
			}
		};
		std::unordered_map<Key, size_t, KeyHash> map;

		// Entity一覧を走査してバッチ化
		for (auto& e : entities_) {
			auto mesh = ecs_.GetComponent<MeshRenderer>(e);
			auto tf = ecs_.GetComponent<Transform>(e);

			auto& mr = engine_.GetMeshRegistry();
			auto meshData = mr.Get(mesh->handle);

			if (!meshData) continue;

			Key key{ meshData->vb.get(), meshData->ib.get() };
			size_t batchIndex{};
			// 既存のバッチ
			if (auto it = map.find(key); it != map.end()) {
				batchIndex = it->second;
			}
			// 新しいバッチ
			else {
				batchIndex = shadow_batches_.size();
				map.emplace(key, batchIndex);
				shadow_batches_.push_back(InstanceBatchShadow{
					.vb = meshData->vb,
					.ib = meshData->ib,
					.indexCount = meshData->indexCount,
					.instances = {},
					.instanceOffset = 0
					});
			}

			// インスタンスデータ追加
			dx3d::InstanceDataShadow ds{};
			ds.world = tf->world;
			shadow_batches_[batchIndex].instances.emplace_back(ds);
		}
	}

	void LightDepthRenderSystem::UpdateBatches()
	{
		// 総インスタンス数
		size_t totalInstance = 0;
		for (auto& b : shadow_batches_) {
			totalInstance += b.instances.size();
		}
		if (totalInstance == 0) { return; } // 描画するものがない

		// インスタンスバッファの作成またはリサイズ
		CreateOrResizeInstanceBufferShadow(totalInstance);

		std::vector<dx3d::InstanceDataShadow> instances;
		instances.reserve(totalInstance);

		size_t cursor = 0;

		// インスタンスデータをインスタンスバッファ用に変換して格納
		for (auto& b : shadow_batches_) {
			b.instanceOffset = cursor;
			for (auto& inst : b.instances) {
				instances.emplace_back(inst);
			}
			cursor += b.instances.size();
		}

		// インスタンスバッファの作成
		{
			dx3d::VertexBufferDesc desc{
				.vertexList = instances.data(),
				.vertexListSize = static_cast<uint32_t>(instances.size() * sizeof(dx3d::InstanceDataShadow)),
				.vertexSize = static_cast<uint32_t>(sizeof(dx3d::InstanceDataShadow))
			};
			instance_buffer_shadow_ = engine_.GetGraphicsDevice().CreateVertexBuffer(desc);
		}
	}

	/**
	 * @brief インスタンスバッファの作成またはリサイズ
	 * @param _requiredInstanceCapacity 必要なインスタンス数
	 */
	void LightDepthRenderSystem::CreateOrResizeInstanceBufferShadow(size_t _requiredInstanceCapacity)
	{
		if (_requiredInstanceCapacity <= instance_buffer_capacity_) { return; }
		instance_buffer_capacity_ = (std::max)(_requiredInstanceCapacity, instance_buffer_capacity_ * 2 + 1);
	}


	/**
	 * @brief シャドウマップに必要なリソースの生成
	 */
	void LightDepthRenderSystem::CreateShadowResources(uint32_t _texHeight, uint32_t _texWidth, uint32_t _lightCount)
	{
		auto& device = engine_.GetGraphicsDevice();

		shadow_dsvs_.resize(_lightCount);

		// テクスチャ配列の作成
		Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTex{};
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = _texWidth;
		texDesc.Height = _texHeight;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = _lightCount;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		device.CreateTexture2D(&texDesc, nullptr, &shadowTex);

		shadow_depth_tex_ = shadowTex;

		// DSVの作成
		for (uint32_t i = 0; i < _lightCount; ++i) {
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.MipSlice = 0;
			dsvDesc.Texture2DArray.FirstArraySlice = i;
			dsvDesc.Texture2DArray.ArraySize = 1;
			device.CreateDepthStencilView(shadowTex.Get(), &dsvDesc, &shadow_dsvs_[i]);
		}

		// SRVの作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = _lightCount;

		device.CreateShaderResourceView(shadowTex.Get(), &srvDesc, &shadow_srvs_);
	}


	/**
	 * @brief Brief シャドウマップ描画パス
	 */
	void LightDepthRenderSystem::RenderShadowPass(ShadowLightEntry _entry, ID3D11DepthStencilView* _dsv)
	{
		auto debugRenderSystem = debug_render_system_.lock();

		auto& deferredContext = engine_.GetDeferredContext();
		auto contextD3D = deferredContext.GetDeferredContext();
		// ライト行列の作成
		auto tf = ecs_.GetComponent<Transform>(_entry.light);
		SpotLight* spotPtr = ecs_.HasComponent<SpotLight>(_entry.light) ? ecs_.GetComponent<SpotLight>(_entry.light) : nullptr;

		// 現在のRTV、DSVを退避
		ID3D11RenderTargetView* prevRTV = nullptr;
		ID3D11DepthStencilView* prevDSV = nullptr;
		contextD3D->OMGetRenderTargets(1, &prevRTV, &prevDSV);

		// シャドウ用DSVをセット
		contextD3D->OMSetRenderTargets(0, nullptr, _dsv);
		contextD3D->ClearDepthStencilView(_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
		deferredContext.SetViewportSize({ static_cast<int32_t>(SHADOW_MAP_WIDTH), static_cast<int32_t>(SHADOW_MAP_HEIGHT) });


		// ライト行列をセット
		{
			CBLightMatrix lm{};
			lm.lightViewProj = _entry.lightViewProj;
			cb_light_matrix_->Update(deferredContext, &lm, sizeof(lm));
			deferredContext.VSSetConstantBuffer(1, *cb_light_matrix_);
		}


		auto psoKey = dx3d::BuildPipelineKey(
			dx3d::VertexShaderKind::ShadowMap,
			dx3d::PixelShaderKind::None,
			dx3d::BlendMode::Opaque,
			dx3d::DepthMode::Default,
			dx3d::RasterMode::SolidBack,
			dx3d::PipelineFlags::Instancing
		);
		// 描画
		for (auto& b : shadow_batches_) {
			const uint32_t instanceCount = static_cast<uint32_t>(b.instances.size());
			if (instanceCount == 0) { continue; }
			engine_.RenderInstanced(*b.vb, *b.ib, *instance_buffer_shadow_, instanceCount, b.instanceOffset, psoKey);
		}

		// 退避していたRTV、DSVを復元
		{
			contextD3D->OMSetRenderTargets(1, &prevRTV, prevDSV);
			if (prevRTV) { prevRTV->Release(); }
			if (prevDSV) { prevDSV->Release(); }
		}

	}
}