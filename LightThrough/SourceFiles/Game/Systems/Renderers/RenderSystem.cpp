/**
 * @file RenderSystem.cpp
 * @brief 描画システム
 * @author Arima Keita
 * @date 2025-09-01
 */

 // ---------- インクルード ---------- //
#include <DirectXMath.h>
#include <Game/Systems/Renderers/RenderSystem.h>
#include <Game/Systems/Renderers/ShadowMapSystem.h>

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
		cb_per_frame_ = device.CreateConstantBuffer({
			sizeof(CBPerFrame),
			nullptr
			});

		cb_per_object_ = device.CreateConstantBuffer({
			sizeof(CBPerObject),
			nullptr
			});

		cb_lighting_ = device.CreateConstantBuffer({
			sizeof(CBLight),
			nullptr
			});
	}

	/**
	 * @brief 更新処理
	 */
	void RenderSystem::Update(float _dt)
	{
		auto& context = engine_->GetDeviceContext();
		auto& device = engine_->GetGraphicsDevice();

		// CameraComponentを持つEntityを取得 memo: 現状カメラは一つだけを想定
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
		// 共通処理
		for (auto e : ecs_.GetEntitiesWithComponent<LightCommon>()) {
			// ライトの総数が多すぎるなら
			if (lightSum >= MAX_LIGHTS) { break; }
			auto& common = ecs_.GetComponent<ecs::LightCommon>(e);
			// 無効状態なら
			if (!common.enabled) { continue; }

			// 方向の取得
			auto& tf = ecs_.GetComponent<ecs::Transform>(e);
			const XMFLOAT3& fwd = tf.GetForward();

			auto& L = lightData.lights[lightSum++];
			L.pos_type = { tf.position.x, tf.position.y, tf.position.z, 0.0f };
			L.dir_range = { fwd.x, fwd.y, fwd.z, 0.0f };
			L.color = {
				common.color.x * common.intensity,
				common.color.y * common.intensity,
				common.color.z * common.intensity,
				1.0f
			};
			L.spotAngles = { 0.0f, 0.0f, 0.0f, 0.0f };

			// スポットライト
			if (ecs_.HasComponent<SpotLight>(e)) {
				// スポットライト
				auto& spot = ecs_.GetComponent<SpotLight>(e);
				L.pos_type.w = 1.0f;
				L.dir_range.w = spot.range;
				L.spotAngles.x = spot.innerCos;
				L.spotAngles.y = spot.outerCos;
			}
			//else if(ecs_.HasComponent<>(e)){}
		}
		// 総数
		lightData.lightCount = lightSum;

		cb_lighting_->Update(context, &lightData, sizeof(lightData));
		context.PSSetConstantBuffer(1, *cb_lighting_); // スロット1

		// バッチ処理
		batches_.clear();	// バッチクリア
		CollectBatches();	// バッチ収集
		// メインパス
		UploadAndDrawBatches();
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
	 * @brief バッチアップロード＆描画
	 *
	 * 各バッチのインスタンスデータをインスタンスバッファにアップロードし、描画する
	 */
	void RenderSystem::UploadAndDrawBatches()
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

		auto& context = engine_->GetDeviceContext();

		// 描画
		for (auto& b : batches_) {
			const uint32_t instanceCount = static_cast<uint32_t>(b.instances.size());
			if (instanceCount == 0) { continue; }

			if (instanceCount == 1) {
				// インスタンスが1つだけなら通常描画
				CBPerObject obj{};
				DirectX::XMMATRIX w = DirectX::XMLoadFloat4x4(&b.instances[0].world);
				//obj.world = DirectX::XMMatrixTranspose(w);
				obj.world = w;	// 非転置
				cb_per_object_->Update(context, &obj, sizeof(obj));
				context.VSSetConstantBuffer(1, *cb_per_object_);
				engine_->Render(*b.vb, *b.ib);
			}
			else {
				// インスタンスが複数ならインスタンス描画
				engine_->RenderInstanced(*b.vb, *b.ib, *instance_buffer_, instanceCount, b.instanceOffset);
			}
		}
	}
}