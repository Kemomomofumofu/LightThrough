/**
 * @file RenderSystem.cpp
 * @brief 描画システム
 * @author Arima Keita
 * @date 2025-09-01
 */

 // ---------- インクルード ---------- //
#include <DirectXMath.h>

#include <DX3D/Graphics/Buffers/ConstantBuffer.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/Meshes/MeshRegistry.h>
#include <DX3D/Graphics/Meshes/Mesh.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Systems/RenderSystem.h>

#include <Game/Components/Camera.h>
#include <Game/Components/Transform.h>
#include <Game/Components/MeshRenderer.h>


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

		// ConstantBuffer作成
		cb_per_frame_ = engine_->GetGraphicsDevice().CreateConstantBuffer({
			sizeof(dx3d::CBPerFrame),
			nullptr
			});

		cb_per_object_ = engine_->GetGraphicsDevice().CreateConstantBuffer({
			sizeof(dx3d::CBPerObject),
			nullptr
			});


		cb_lighting_ = engine_->GetGraphicsDevice().CreateConstantBuffer({
			sizeof(dx3d::LightingCB),
			nullptr
			});

	}

	/**
	 * @brief 更新処理
	 * @param _dt デルタタイム
	 */
	void RenderSystem::Update(float _dt)
	{

		auto& context = engine_->GetDeviceContext();
		auto& device = engine_->GetGraphicsDevice();

		// todo: Entity自体に有効かどうかを持たせるべきかも、そこで参照保持でUpdateでGet~~はしないようにしたい。
			// CameraComponentを持つEntityを取得 memo: 現状カメラは一つだけを想定
			auto camEntities = ecs_.GetEntitiesWithComponent<Camera>();
			if (camEntities.empty()) {
				GameLogWarning("CameraComponentを持つEntityが存在しないため、描画をスキップ");
				return;
			}
		auto& cam = ecs_.GetComponent<Camera>(camEntities[0]);

		dx3d::CBPerFrame cbPerFrameData{};
		cbPerFrameData.view = cam.view;
		cbPerFrameData.proj = cam.proj;

		// 定数バッファ更新
		cb_per_frame_->Update(context, &cbPerFrameData, sizeof(cbPerFrameData));
		context.VSSetConstantBuffer(0, *cb_per_frame_);	// 頂点シェーダーのスロット0にセット

		// memo: 今は定数。todo: ライトコンポーネントを持つオブジェクトから引っ張ってくる。
		dx3d::LightingCB cbLightingData{
			.lightDirWS = { 0.5f, -1.0f, 0.2f },
			.lightColor = { 0.9f, 0.9f, 1.0f },
			.ambientColor = { 0.175f, 0.25f, 0.2f }
		};


		cb_lighting_->Update(context, &cbLightingData, sizeof(cbLightingData));
		context.PSSetConstantBuffer(1, *cb_lighting_);	// ピクセルシェーダーのスロット1にセット

		// バッチ処理
		batches_.clear();	// バッチクリア
		CollectBatches();	// バッチ収集
		UploadAndDrawBatches();	// バッチアップロード＆描画
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

		// Entity一覧を走査してバッチ化 // [ToDo] 毎フレーム全Entityに対して処理するのはあまりにも重すぎなので、差分更新とかにしたい。
		for (auto& e : entities_) {
			auto& mesh = ecs_.GetComponent<MeshRenderer>(e);
			auto& tf = ecs_.GetComponent<ecs::Transform>(e);

			auto& mr = engine_->GetMeshRegistry();
			auto meshData = mr.Get(mesh.handle);

			if (!meshData) continue;

			Key key{ meshData->vb.get(), meshData->ib.get() };
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
		instance_buffer_capacity_ = max(_requiredInstanceCapacity, instance_buffer_capacity_ * 2 + 1);
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
				dx3d::CBPerObject obj{};
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
