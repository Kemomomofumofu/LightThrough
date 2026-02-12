/**
 * @file SpriteRenderSystem.cpp
 * @brief スプライト描画システム
 */

 // ---------- インクルード ---------- //
#include <DirectXMath.h>

#include <Game/Systems/Renderers/SpriteRenderSystem.h>

#include <DX3D/Graphics/Buffers/ConstantBuffer.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/Meshes/MeshRegistry.h>
#include <DX3D/Graphics/Meshes/Mesh.h>
#include <DX3D/Graphics/GraphicslogUtils.h>

#include <DX3D/Graphics/Textures/TextureRegistry.h>
#include <DX3D/Graphics/Textures/Texture.h>
#include <DX3D/Graphics/Textures/TextureHandle.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Core/Transform.h>
#include <Game/Components/Render/SpriteRenderer.h>

#include <d3d11.h>
#include <unordered_map>
#include <algorithm>

namespace {
	struct CBPerFrame {
		DirectX::XMFLOAT4X4 view;	// ビュー行列
		DirectX::XMFLOAT4X4 proj;	// プロジェクション行列
	};

	DirectX::XMFLOAT4X4 MakeIdentity()
	{
		using namespace DirectX;
		XMFLOAT4X4 m{};
		XMStoreFloat4x4(&m, XMMatrixIdentity());
		return m;
	}


	DirectX::XMFLOAT4X4 MakeOrthoUI(float _w, float _h)
	{
		using namespace DirectX;
		if (_w <= 0.0f || _h <= 0.0f) {
			return MakeIdentity();
		}

		const float halfW = _w * 0.5f;
		const float halfH = _h * 0.5f;

		XMFLOAT4X4 m{};
		XMStoreFloat4x4(&m, XMMatrixOrthographicOffCenterLH(
			-halfW, halfW,
			halfH, -halfH,
			-1.0f, 1.0f
		));
		return m;
	}

	static DirectX::XMFLOAT4X4 MakeUISpriteWorldCenteredOrigin(
		float px, float py,           // pivot位置の座標（画面中心原点）
		float w, float h,             // ピクセルサイズ
		float pivotX, float pivotY,   // 0..1
		float z = 0.0f)
	{
		using namespace DirectX;

		// pivot=(0.5,0.5)なら補正0で中心位置になる
		const float cx = px + (0.5f - pivotX) * w;
		const float cy = py + (0.5f - pivotY) * h;

		XMMATRIX S = XMMatrixScaling(w, h, 1.0f);
		XMMATRIX T = XMMatrixTranslation(cx, cy, z);

		XMFLOAT4X4 out{};
		XMStoreFloat4x4(&out, S * T);
		return out;
	}

} // namespace anonymous

namespace ecs {

	SpriteRenderSystem::SpriteRenderSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
	}

	void SpriteRenderSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<SpriteRenderer>());
		ecs_.SetSystemSignature<SpriteRenderSystem>(signature);

		auto& device = engine_.GetGraphicsDevice();

		// vsスロット0
		cb_per_frame_ = device.CreateConstantBuffer({
			sizeof(CBPerFrame),
			nullptr
			});

		// UI用サンプラ（Linear + Clamp）
		ID3D11Device* d3d = device.GetD3DDevice().Get();
		if (d3d) {
			D3D11_SAMPLER_DESC sd{};
			sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			sd.MaxAnisotropy = 1;
			sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			sd.MinLOD = 0;
			sd.MaxLOD = D3D11_FLOAT32_MAX;

			HRESULT hr = d3d->CreateSamplerState(&sd, sampler_linear_clamp_.GetAddressOf());
			if (FAILED(hr)) {
				DebugLogWarning("[SpriteRenderSystem] SamplerStateの作成に失敗\n");
			}
		}

		// パイプラインKey生成
		pso_key_ = dx3d::BuildPipelineKey(
			dx3d::VertexShaderKind::Instanced,
			dx3d::PixelShaderKind::Sprite,
			dx3d::BlendMode::Alpha,
			dx3d::DepthMode::Disable,
			dx3d::RasterMode::SolidNone,
			dx3d::PipelineFlags::Instancing
		);

		// メッシュデータ取得
		quad_mesh_ = engine_.GetMeshRegistry().GetByName("Quad");
	}

	void SpriteRenderSystem::Update(float _dt)
	{
		auto& context = engine_.GetDeferredContext();

		// スクリーン
		const auto& screen = engine_.GetScreenSize();

		// 重要: 幅/高さが無効（0）だと Orthographic matrix 生成でアサートするため描画をスキップする
		if (screen.width <= 0 || screen.height <= 0) {
			DebugLogWarning("[SpriteRenderSystem] 無効なスクリーンサイズのためスプライト描画をスキップ\n");
			return;
		}

		CBPerFrame cb{};
		cb.view = MakeIdentity();
		cb.proj = MakeOrthoUI(static_cast<float>(screen.width), static_cast<float>(screen.height));

		cb_per_frame_->Update(context, &cb, sizeof(cb));
		context.VSSetConstantBuffer(0, *cb_per_frame_);

		// バッチ処理
		batches_.clear();
		CollectBatches();
		UpdateBatches();

		// UIパス
		RenderSpritePass();
	}

	void SpriteRenderSystem::CollectBatches()
	{
		struct Key {
			dx3d::VertexBuffer* vb{};
			dx3d::IndexBuffer* ib{};
			dx3d::PipelineKey psoKey{};
			dx3d::TextureHandle texHandle{};
			int layer{};
			bool operator==(const Key& _o) const noexcept {
				return vb == _o.vb && ib == _o.ib && psoKey == _o.psoKey && texHandle == _o.texHandle && layer == _o.layer;
			}
		};

		struct KeyHash {
			size_t operator()(const Key& k) const noexcept {
				size_t h1 = std::hash<void*>()(k.vb);
				size_t h2 = std::hash<void*>()(k.ib);
				size_t h3 = dx3d::PipelineKeyHash()(k.psoKey);
				size_t h4 = std::hash<int>()(k.texHandle.id);
				size_t h5 = std::hash<int>()(k.layer);
				return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
			}
		};

		std::unordered_map<Key, size_t, KeyHash> map;

		for (auto& e : entities_) {
			auto tf = ecs_.GetComponent<Transform>(e);
			auto spr = ecs_.GetComponent<SpriteRenderer>(e);

			const dx3d::TextureHandle handle = spr->handle;
			if (!handle.IsValid()) continue;

			const int layer = spr->layer;

			Key key{
				quad_mesh_->vb.get(),
				quad_mesh_->ib.get(),
				pso_key_,
				handle,
				layer
			};

			// バッチ検索または新規作成
			size_t batchIndex{};
			if (auto it = map.find(key); it != map.end()) {
				batchIndex = it->second;
			}
			else {
				batchIndex = batches_.size();
				map.emplace(key, batchIndex);

				InstanceBatchSprite b{};
				b.vb = quad_mesh_->vb;
				b.ib = quad_mesh_->ib;
				b.indexCount = quad_mesh_->indexCount;
				b.pipelineKey = pso_key_;
				b.textureHandle = handle;
				b.layer = layer;
				batches_.push_back(std::move(b));
			}

			dx3d::InstanceDataSprite inst{};
			const float cx = tf->position.x;
			const float cy = tf->position.y;
			const float w = spr->size.x * tf->scale.x;
			const float h = spr->size.y * tf->scale.y;

			inst.world = MakeUISpriteWorldCenteredOrigin(
				tf->position.x, tf->position.y,
				w, h,
				spr->pivot.x, spr->pivot.y,
				0.0f
			);
			inst.color = spr->color;
			batches_[batchIndex].instances.push_back(inst);
		}

		// ソート
		std::sort(batches_.begin(), batches_.end(),
			[](const InstanceBatchSprite& _a, const InstanceBatchSprite& _b) {
				if (_a.layer != _b.layer) { return _a.layer < _b.layer; } // 小さいほど手前に描画
				return _a.textureHandle < _b.textureHandle;
			});
	}

	void SpriteRenderSystem::UpdateBatches()
	{
		size_t totalInstance = 0;
		for (auto& b : batches_) {
			totalInstance += b.instances.size();
		}
		if (totalInstance == 0) return;

		CreateOrResizeInstanceBuffer(totalInstance);

		std::vector<dx3d::InstanceDataSprite> instances;
		instances.reserve(totalInstance);

		size_t cursor = 0;
		for (auto& b : batches_) {
			b.instanceOffset = cursor;
			instances.insert(instances.end(), b.instances.begin(), b.instances.end());
			cursor += b.instances.size();
		}

		dx3d::VertexBufferDesc desc{
			.vertexList = instances.data(),
			.vertexListSize = static_cast<uint32_t>(instances.size() * sizeof(dx3d::InstanceDataSprite)),
			.vertexSize = static_cast<uint32_t>(sizeof(dx3d::InstanceDataSprite))
		};
		instance_buffer_ = engine_.GetGraphicsDevice().CreateVertexBuffer(desc);
	}

	void SpriteRenderSystem::RenderSpritePass()
	{
		auto& context = engine_.GetDeferredContext();

		// sampler
		if (sampler_linear_clamp_) {
			ID3D11SamplerState* s = sampler_linear_clamp_.Get();
			context.PSSetSamplers(0, 1, &s);
		}

		auto& texReg = engine_.GetTextureRegistry();

		for (auto& b : batches_) {
			if (b.instances.empty()) continue;

			// テクスチャ取得
			auto tex = texReg.Get(b.textureHandle);
			if (!tex) continue;

			ID3D11ShaderResourceView* srv = tex->srv_.Get();
			context.PSSetShaderResources(0, 1, &srv);

			engine_.RenderInstanced(*b.vb, *b.ib, *instance_buffer_, static_cast<uint32_t>(b.instances.size()), static_cast<uint32_t>(b.instanceOffset), b.pipelineKey);

			// SRVを外す
			ID3D11ShaderResourceView* nullSrv = nullptr;
			context.PSSetShaderResources(0, 1, &nullSrv);
		}
	}

	void SpriteRenderSystem::CreateOrResizeInstanceBuffer(size_t _requiredInstanceCapacity)
	{
		if (_requiredInstanceCapacity <= instance_buffer_capacity_) return;
		instance_buffer_capacity_ = (std::max)(_requiredInstanceCapacity, instance_buffer_capacity_ * 2 + 1);
	}

}