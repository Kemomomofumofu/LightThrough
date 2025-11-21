/**
 * @file RenderSystem.cpp
 * @brief 描画システム
 * @author Arima Keita
 * @date 2025-09-01
 */

 // ---------- インクルード ---------- //
#include <DirectXMath.h>
#include <Game/Systems/Renderers/RenderSystem.h>
#include <Game/Systems/Renderers/DebugRenderSystem.h>

#include <DX3D/Graphics/Buffers/ConstantBuffer.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/Meshes/MeshRegistry.h>
#include <DX3D/Graphics/Meshes/Mesh.h>
#include <DX3D/Graphics/GraphicslogUtils.h>
#include <DX3D/Graphics/PipelineKey.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Camera.h>
#include <Game/Components/Transform.h>
#include <Game/Components/MeshRenderer.h>
#include <Game/Components/Light.h>

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
	RenderSystem::RenderSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
	}

	void RenderSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<MeshRenderer>());
		ecs_.SetSystemSignature<RenderSystem>(signature);

		auto& device = engine_->GetGraphicsDevice();
		// ConstantBuffer作成
		cb_per_frame_ = device.CreateConstantBuffer({	// vsスロット0
			sizeof(CBPerFrame),
			nullptr
			});

		cb_per_object_ = device.CreateConstantBuffer({	// vsスロット1
			sizeof(CBPerObject),
			nullptr
			});

		cb_light_matrix_ = device.CreateConstantBuffer({	// vsスロット2
			sizeof(CBLightMatrix),
			nullptr
			});

		cb_lighting_ = device.CreateConstantBuffer({	// psスロット0
			sizeof(CBLight),
			nullptr
			});

		{
			D3D11_SAMPLER_DESC sd{};
			sd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			sd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
			sd.BorderColor[0] = sd.BorderColor[1] = sd.BorderColor[2] = sd.BorderColor[3] = 1.0f;
			engine_->GetGraphicsDevice().GetD3DDevice()->CreateSamplerState(&sd, &shadow_sampler_);
		}

		CreateShadowResources(SHADOW_MAP_SIZE);
	}

	/**
	 * @brief 更新処理
	 */
	void RenderSystem::Update(float _dt)
	{
		auto& context = engine_->GetDeviceContext();
		auto& device = engine_->GetGraphicsDevice();

		// Camera取得 memo: 現状カメラは一つだけを想定
		auto camEntities = ecs_.GetEntitiesWithComponent<Camera>();
		if (camEntities.empty()) {
			GameLogWarning("CameraComponentを持つEntityが存在しないため、描画をスキップ");
			return;
		}
		// カメラ
		auto& cam = ecs_.GetComponent<Camera>(camEntities[0]);
		CBPerFrame cbPerFrameData{};
		cbPerFrameData.view = cam.view;
		cbPerFrameData.proj = cam.proj;

		// 定数バッファ更新
		cb_per_frame_->Update(context, &cbPerFrameData, sizeof(cbPerFrameData));
		context.VSSetConstantBuffer(0, *cb_per_frame_);	// 頂点シェーダーのスロット0にセット

		// ---------- ライト処理 ---------- // 
		CBLight lightData{};
		int lightSum = 0;

		// memo: 仮で1つまで
		Entity firstLight{};
		XMMATRIX lightVP = XMMatrixIdentity();

		// 共通処理
		for (auto e : ecs_.GetEntitiesWithComponent<LightCommon>()) {
			// ライトの総数が多すぎるなら
			if (lightSum >= MAX_LIGHTS) { break; }
			auto& common = ecs_.GetComponent<LightCommon>(e);
			// 無効状態なら
			if (!common.enabled) { continue; }

			// 方向の取得
			auto& tf = ecs_.GetComponent<Transform>(e);
#ifdef _DEBUG || DEBUG
			auto debugRender = ecs_.GetSystem<DebugRenderSystem>();
			debugRender->DrawCube(tf, { 1, 0, 0, 1 });
#endif // _DEBUG || DEBUG
			SpotLight* spotPtr = nullptr;
			if (ecs_.HasComponent<SpotLight>(e)) {
				spotPtr = &ecs_.GetComponent<SpotLight>(e);
			}

			// パック
			lightData.lights[lightSum++] = BuildLightCPU(tf, common, spotPtr);
			lightVP = BuildLightViewProj(tf, spotPtr, 0.1f);

		}
		lightData.lightCount = lightSum;

		// ライト行列CB（PS側でシャドウ計算に使用）
		if (lightSum > 0) {
			CBLightMatrix cbLM{};
			cbLM.lightViewProj = lightVP; // 非転置
			cb_light_matrix_->Update(context, &cbLM, sizeof(cbLM));
			context.VSSetConstantBuffer(2, *cb_light_matrix_); // スロット2
		}


		// バッチ処理
		batches_.clear();	// バッチクリア
		CollectBatches();	// バッチ収集
		UpdateBatches();	// バッチ更新

		// ---------- シャドウパス（深度のみ） ---------- //
		if (lightSum > 0 && shadow_dsv_) {
			RenderShadowPass(lightVP);

			// メインパス用のカメラCBを復元
			cb_per_frame_->Update(context, &cbPerFrameData, sizeof(cbPerFrameData));
			context.VSSetConstantBuffer(0, *cb_per_frame_);

			// PSへシャドウSRVをバインド
			auto contextD3D = context.GetDeviceContext();
			ID3D11ShaderResourceView* srvs[1] = { shadow_srv_.Get() };
			contextD3D->PSSetShaderResources(0, 1, srvs);
			// シャドウサンプラーセット
			ID3D11SamplerState* samps[1] = { shadow_sampler_.Get() };
			contextD3D->PSSetSamplers(0, 1, samps);
		}

		// ---------- メインパス ---------- //
		RenderMainPass(lightData);

	}

	/**
	 * @brief エンティティ破棄イベント
	 */
	void RenderSystem::OnEntityDestroyed(Entity _entity)
	{

	}

	/**
	 * @brief バッチ収集
	 *
	 * Meshの頂点バッファとインデックスバッファが同じものをまとめてバッチ化する
	 * インスタンスデータは各EntityのTransformからワールド行列を取得して格納する
	 */
	void RenderSystem::CollectBatches()
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

		// Entity一覧を走査してバッチ化 // todo: 毎フレーム全Entityに対して処理するのはあまりにも無駄なので、差分更新とかにしたい。
		for (auto& e : entities_) {
			auto& mesh = ecs_.GetComponent<MeshRenderer>(e);
			auto& tf = ecs_.GetComponent<ecs::Transform>(e);

			auto& mr = engine_->GetMeshRegistry();
			auto meshData = mr.Get(mesh.handle);

			if (!meshData) continue;

			struct Key key { meshData->vb.get(), meshData->ib.get() };
			size_t batchIndex{};
			// 既存のバッチ
			if (auto it = map.find(key); it != map.end()) {
				batchIndex = it->second;
			}
			// 新しいバッチ
			else {
				batchIndex = batches_.size();
				map.emplace(key, batchIndex);
				batches_.push_back(InstanceBatch{
					.vb = meshData->vb,
					.ib = meshData->ib,
					.indexCount = meshData->indexCount,
					.instances = {},
					.instanceOffset = 0
					});
			}

			// インスタンスデータ追加
			dx3d::InstanceData d{};
			d.world = tf.world;
			batches_[batchIndex].instances.emplace_back(d);
		}
	}

	void RenderSystem::UpdateBatches()
	{
		// 総インスタンス数
		size_t totalInstance = 0;
		for (auto& b : batches_) {
			totalInstance += b.instances.size();
		}
		if (totalInstance == 0) { return; } // 描画するものがない

		// インスタンスバッファの作成またはリサイズ
		CreateOrResizeInstanceBuffer(totalInstance);

		std::vector<dx3d::InstanceData> instances;
		instances.reserve(totalInstance);

		size_t cursor = 0;

		// インスタンスデータをインスタンスバッファ用に変換して格納
		for (auto& b : batches_) {
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
				.vertexListSize = static_cast<uint32_t>(instances.size() * sizeof(dx3d::InstanceData)),
				.vertexSize = static_cast<uint32_t>(sizeof(dx3d::InstanceData))
			};
			instance_buffer_ = engine_->GetGraphicsDevice().CreateVertexBuffer(desc);
		}
	}

	void RenderSystem::RenderMainPass(CBLight& _lightData)
	{
		auto& context = engine_->GetDeviceContext();
		
		// ライティングCB更新
		cb_lighting_->Update(context, &_lightData, sizeof(_lightData));
		context.PSSetConstantBuffer(0, *cb_lighting_); // スロット0

		// 描画
		auto key = dx3d::BuildPipelineKey(false);
		for (auto& b : batches_) {
			const uint32_t instanceCount = static_cast<uint32_t>(b.instances.size());
			if (instanceCount == 0) { continue; }

			// 描画
			engine_->RenderInstanced(*b.vb, *b.ib, *instance_buffer_, instanceCount, b.instanceOffset, key);
		}

	}

	/**
	 * @brief インスタンスバッファの作成またはリサイズ
	 * @param _requiredInstanceCapacity 必要なインスタンス数
	 */
	void RenderSystem::CreateOrResizeInstanceBuffer(size_t _requiredInstanceCapacity)
	{
		if (_requiredInstanceCapacity <= instance_buffer_capacity_) { return; }
		instance_buffer_capacity_ = (std::max)(_requiredInstanceCapacity, instance_buffer_capacity_ * 2 + 1);
	}

	/**
	 * @brief シャドウマップに必要なリソースの生成
	 * @param _size: テクスチャのサイズ
	 */
	void RenderSystem::CreateShadowResources(uint32_t _size)
	{
		auto& device = engine_->GetGraphicsDevice();

		// シャドウマップ用テクスチャ作成
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = _size;
		texDesc.Height = _size;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		device.CreateTexture2D(&texDesc, nullptr, &shadow_depth_tex_);

		// DSV 作成
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		device.CreateDepthStencilView(shadow_depth_tex_.Get(), &dsvDesc, &shadow_dsv_);

		// SRV 作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		device.CreateShaderResourceView(shadow_depth_tex_.Get(), &srvDesc, &shadow_srv_);
	}

	void RenderSystem::RenderShadowPass(const DirectX::XMMATRIX& _lightViewProj)
	{
		auto& contextWrap = engine_->GetDeviceContext();
		auto contextD3D = contextWrap.GetDeviceContext();

		// SRVのアンバインド
		{
			ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
			contextD3D->PSSetShaderResources(0, 1, nullSRV);
			contextD3D->VSSetShaderResources(0, 1, nullSRV);
		}

		// 現在のRTV、DSVを退避
		ID3D11RenderTargetView* prevRTV = nullptr;
		ID3D11DepthStencilView* prevDSV = nullptr;
		contextD3D->OMGetRenderTargets(1, &prevRTV, &prevDSV);

		// シャドウ用DSVをセット
		contextD3D->OMSetRenderTargets(0, nullptr, shadow_dsv_.Get());
		contextD3D->ClearDepthStencilView(shadow_dsv_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		contextWrap.SetViewportSize({ static_cast<int32_t>(SHADOW_MAP_SIZE), static_cast<int32_t>(SHADOW_MAP_SIZE) });

		// ライト行列をセット
		{
			CBLightMatrix lm{};
			lm.lightViewProj = _lightViewProj;
			cb_light_matrix_->Update(contextWrap, &lm, sizeof(lm));
			contextWrap.VSSetConstantBuffer(0, *cb_light_matrix_); // slot0
		}

		auto key = dx3d::BuildPipelineKey(true);
		// 描画
		for (auto& b : batches_) {
			const uint32_t instanceCount = static_cast<uint32_t>(b.instances.size());
			if (instanceCount == 0) { continue; }
			engine_->RenderInstanced(*b.vb, *b.ib, *instance_buffer_, instanceCount, b.instanceOffset, key);
		}


		{
			contextD3D->OMSetRenderTargets(1, &prevRTV, prevDSV);
			if (prevRTV) { prevRTV->Release(); }
			if (prevDSV) { prevDSV->Release(); }
		}

	}
}