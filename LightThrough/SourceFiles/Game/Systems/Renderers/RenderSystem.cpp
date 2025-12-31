/**
 * @file RenderSystem.cpp
 * @brief 描画システム
 * @author Arima Keita
 * @date 2025-09-01
 */

 // ---------- インクルード ---------- //
#include <DirectXMath.h>
#include <Game/Systems/Renderers/RenderSystem.h>
#include <Game/Systems/Renderers/LightDepthRenderSystem.h>
#include <Game/Systems/Renderers/DebugRenderSystem.h>

#include <DX3D/Graphics/Buffers/ConstantBuffer.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/Meshes/MeshRegistry.h>
#include <DX3D/Graphics/Meshes/Mesh.h>
#include <DX3D/Graphics/GraphicslogUtils.h>
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

		cb_lighting_ = device.CreateConstantBuffer({	// psスロット0
			sizeof(CBLight),
			nullptr
			});

	}

	/**
	 * @brief 更新処理
	 */
	void RenderSystem::Update(float _dt)
	{
		auto& context = engine_->GetDeferredContext();
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


		// 他システムからライト深度情報を取得
		auto shadowSystem = ecs_.GetSystem<LightDepthRenderSystem>();
		auto shadowSRV = shadowSystem->GetShadowMapSRV();
		auto shadowSampler = shadowSystem->GetShadowSampler();

		// PSにリソースをバインド
		if (shadowSRV) {
			ID3D11ShaderResourceView* srvs[1] = { shadowSRV };
			context.PSSetShaderResources(0, 1, srvs);	// t0
			ID3D11SamplerState* samps[1] = { shadowSampler };
			context.PSSetSamplers(0, 1, samps);			// s0
		}

		// ---------- ライト処理 ---------- // 
		// 共通処理
		CBLight lightData{};
		int lightSum = 0;
		for (auto& e : ecs_.GetEntitiesWithComponent<LightCommon>()) {
			if (lightSum >= MAX_LIGHTS) { break; }	// ライトの総数が多すぎるなら

			auto& common = ecs_.GetComponent<LightCommon>(e);
			if (!common.enabled) { continue; }	// 無効状態なら

			SpotLight* spotPtr = nullptr;
			if (ecs_.HasComponent<SpotLight>(e)) {
				spotPtr = &ecs_.GetComponent<SpotLight>(e);
			}

			// 方向の取得
			auto& tf = ecs_.GetComponent<Transform>(e);
#ifdef _DEBUG || DEBUG

			// ライトの位置に丸を描画。
			auto debugRender = ecs_.GetSystem<DebugRenderSystem>();
			debugRender->DrawSphere(tf, { 1, 0, 0, 1 });
#endif // _DEBUG || DEBUG

			// パック
			lightData.lights[lightSum] = BuildLightCPU(tf, common, spotPtr);

			int shadowIndex = -1;
			DirectX::XMMATRIX lightMatrix;
			if (shadowSystem->GetShadowInfo(e, shadowIndex, lightMatrix)) {
				lightData.lights[lightSum].spotAngles_shadowIndex.z = shadowIndex;
				lightData.lightViewProj[lightSum] = lightMatrix;
			}
			else {
				lightData.lights[lightSum].spotAngles_shadowIndex.z = -1.0f;	// シャドウマップなし
			}

			++lightSum;
		}
		lightData.lightCount = lightSum;

		// バッチ処理
		opaque_batches_.clear();	// バッチクリア
		transparent_batches_.clear();
		auto camPos = ecs_.GetComponent<Transform>(camEntities[0]).position;
		CollectBatches(camPos);	// バッチ収集
		UpdateBatches();	// バッチ更新

		// メインパス
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
	void RenderSystem::CollectBatches(const DirectX::XMFLOAT3& _camPos)
	{
		struct Key {
			dx3d::VertexBuffer* vb{};
			dx3d::IndexBuffer* ib{};
			dx3d::PipelineKey psoKey{};
			bool operator==(const Key& o) const noexcept { return vb == o.vb && ib == o.ib; }
		};

		struct KeyHash {
			size_t operator()(const Key& _k) const noexcept {
				size_t h1 = std::hash<void*>()(_k.vb);
				size_t h2 = std::hash<void*>()(_k.ib);
				size_t h3 = dx3d::PipelineKeyHash()(_k.psoKey);
				return h1 ^ (h2 << 1) ^ (h3 << 2);
			}
		};
		std::unordered_map<Key, size_t, KeyHash> map;

		// Entity一覧を走査してバッチ化 // todo: 毎フレーム全Entityに対して処理するのはあまりにも無駄なので、差分更新とかにしたい。
		for (auto& e : entities_) {
			auto& mesh = ecs_.GetComponent<MeshRenderer>(e);
			auto& tf = ecs_.GetComponent<ecs::Transform>(e);

			auto& mr = engine_->GetMeshRegistry();
			auto meshData = mr.Get(mesh.handle);

			// todo: マテリアルからpsoを取得するつくりにする
			// auto& material = ecs_.GetComponent<Material>(e);
			auto psoKey = dx3d::BuildPipelineKey(false, dx3d::BlendMode::Alpha);

			if (!meshData) continue;


			std::vector<InstanceBatchMain>* targetBatches = nullptr;
			if (psoKey.GetBlend() == dx3d::BlendMode::Opaque) {
				targetBatches = &opaque_batches_;
			}
			else {
				targetBatches = &transparent_batches_;
			}

			struct Key key { meshData->vb.get(), meshData->ib.get(), psoKey };
			size_t batchIndex{};
			// 既存のバッチ
			if (auto it = map.find(key); it != map.end()) {
				batchIndex = it->second;
			}
			// 新しいバッチ
			else {
				batchIndex = targetBatches->size();
				map.emplace(key, batchIndex);
				targetBatches->push_back(InstanceBatchMain{
					.vb = meshData->vb,
					.ib = meshData->ib,
					.indexCount = meshData->indexCount,
					.instances = {},
					.instanceOffset = 0,
					.key = psoKey,
					});
			}

			// インスタンスデータ追加
			dx3d::InstanceDataMain dm{};
			dm.world = tf.world;
			dm.color = { 1, 1, 1, 1 };	// todo: 色実装次第、参照するように
			(*targetBatches)[batchIndex].instances.emplace_back(dm);

			// 半透明なら
			if(psoKey.GetBlend() == dx3d::BlendMode::Alpha) {
				DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&tf.position);
				DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&_camPos);
				float distance = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(pos, camPos)));
				(*targetBatches)[batchIndex].sortKey = distance;
			}

		}
	}

	void RenderSystem::UpdateBatches()
	{
		// 総インスタンス数
		size_t totalInstance = 0;
		for (auto& b : opaque_batches_) {
			totalInstance += b.instances.size();
		}
		for (auto& b : transparent_batches_) {
			totalInstance += b.instances.size();
		}
		if (totalInstance == 0) { return; } // 描画するものがない

		// インスタンスバッファの作成またはリサイズ
		CreateOrResizeInstanceBufferMain(totalInstance);

		std::vector<dx3d::InstanceDataMain> instances;
		instances.reserve(totalInstance);

		size_t cursor = 0;

		// インスタンスデータをインスタンスバッファ用に変換して格納
		for (auto& b : opaque_batches_) {
			b.instanceOffset = cursor;
			instances.insert(instances.end(), b.instances.begin(), b.instances.end());
			cursor += b.instances.size();
		}
		for (auto& b : transparent_batches_) {
			b.instanceOffset = cursor;
			instances.insert(instances.end(), b.instances.begin(), b.instances.end());
			cursor += b.instances.size();
		}

		// インスタンスバッファの作成
		{
			dx3d::VertexBufferDesc desc{
				.vertexList = instances.data(),
				.vertexListSize = static_cast<uint32_t>(instances.size() * sizeof(dx3d::InstanceDataMain)),
				.vertexSize = static_cast<uint32_t>(sizeof(dx3d::InstanceDataMain))
			};
			instance_buffer_main_ = engine_->GetGraphicsDevice().CreateVertexBuffer(desc);
		}
	}

	void RenderSystem::RenderMainPass(CBLight& _lightData)
	{
		auto& context = engine_->GetDeferredContext();

		// ライティングCB更新
		cb_lighting_->Update(context, &_lightData, sizeof(_lightData));
		context.PSSetConstantBuffer(0, *cb_lighting_); // スロット0

		// 不透明オブジェクトのソート
		std::sort(opaque_batches_.begin(), opaque_batches_.end(),
			[](const auto& _a, const auto& _b) {
				return _a.key < _b.key;
			});
		// 描画
		for (auto& b : opaque_batches_) {
			if (b.instances.empty()) { continue; }

			// 描画
			engine_->RenderInstanced(*b.vb, *b.ib, *instance_buffer_main_, b.instances.size(), b.instanceOffset, b.key);
		}

		// 透明オブジェクトのソート（カメラから遠い順）
		std::sort(transparent_batches_.begin(), transparent_batches_.end(),
			[](const auto& _a, const auto& _b) {
				return _a.sortKey > _b.sortKey; // 降順ソート
			});
		for (auto& b : transparent_batches_) {
			if (b.instances.empty()) { continue; }
			// 描画
			engine_->RenderInstanced(*b.vb, *b.ib, *instance_buffer_main_, b.instances.size(), b.instanceOffset, b.key);
		}


	}

	/**
	 * @brief インスタンスバッファの作成またはリサイズ
	 * @param _requiredInstanceCapacity 必要なインスタンス数
	 */
	void RenderSystem::CreateOrResizeInstanceBufferMain(size_t _requiredInstanceCapacity)
	{
		if (_requiredInstanceCapacity <= instance_buffer_capacity_) { return; }
		instance_buffer_capacity_ = (std::max)(_requiredInstanceCapacity, instance_buffer_capacity_ * 2 + 1);
	}
}