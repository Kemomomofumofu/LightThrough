/**
 * @file GraphicsEngine.cpp
 * @brief グラフィックスエンジンクラスの定義
 * @author Arima Keita
 * @date 2025-07-11
 */

 /*---------- インクルード ----------*/
#include <fstream>

#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/Buffers/VertexBuffer.h>
#include <DX3D/Graphics/Buffers/IndexBuffer.h>

#include <DX3D/Graphics/Meshes/PrimitiveFactory.h>
#include <DX3D/Graphics/Textures/TextureRegistry.h>

#include <Debug/Debug.h>

namespace dx3d {
	//! @brief コンストラクタ
	GraphicsEngine::GraphicsEngine(const GraphicsEngineDesc& _desc)
		: Base(_desc.base)
	{
		graphics_device_ = std::make_shared<GraphicsDevice>(GraphicsDeviceDesc{ logger_ });
		
		auto& device = *graphics_device_;
		deferred_context_ = device.CreateDeviceContext();

		// シェーダーキャッシュの生成
		shader_cache_ = device.CreateShaderCache({});

		// パイプラインキャッシュの生成
		pipeline_cache_ = device.CreatePipelineCache({*shader_cache_});

		// メッシュレジストリの生成
		mesh_registry_ = std::make_unique<MeshRegistry>();
		
		// Meshを事前に生成しておく
		PrimitiveFactory::CreateCube(device, *mesh_registry_);
		PrimitiveFactory::CreateSphere(device, *mesh_registry_);
		PrimitiveFactory::CreateQuad(device, *mesh_registry_);
		//PrimitiveFactory::CreateLine(device, *mesh_registry_);

		// テクスチャレジストリの生成
		texture_registry_ = std::make_unique<texture::TextureRegistry>(*graphics_device_->GetD3DDevice().Get());
	}

	//! @brief デストラクタ
	GraphicsEngine::~GraphicsEngine()
	{
	}

	//! @brief グラフィックスデバイス取得
	GraphicsDevice& GraphicsEngine::GetGraphicsDevice() noexcept
	{
		return *graphics_device_;
	}
	//! @brief デバイスコンテキスト取得
	DeviceContext& GraphicsEngine::GetDeferredContext() noexcept
	{
		return *deferred_context_;
	}
	//! @brief スワップチェイン設定
	void GraphicsEngine::SetSwapChain(SwapChain& _swapChain)
	{
		swap_chain_ = &_swapChain;
	}
	//! @brief メッシュレジストリ取得
	MeshRegistry& GraphicsEngine::GetMeshRegistry() noexcept
	{
		return *mesh_registry_;
	}

	//! @brief 描画開始処理
	void GraphicsEngine::BeginFrame()
	{
		auto& context = *deferred_context_;
		DirectX::XMFLOAT4 clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
#ifdef _DEBUG || DEBUG
		clearColor = { 0.2, 0.3f, 0.5f, 1.0f };
#endif
		context.ClearAndSetBackBuffer(*swap_chain_, clearColor);	// 初期色でクリア
	}


	//! @brief 描画処理
	void GraphicsEngine::Render(VertexBuffer& _vb, IndexBuffer& _ib, PipelineKey _key)
	{
		auto pso = pipeline_cache_->GetOrCreate(_key);
		deferred_context_->SetGraphicsPipelineState(*pso);
		deferred_context_->SetViewportSize(swap_chain_->GetSize());
		deferred_context_->SetVertexBuffer(_vb);
		deferred_context_->SetIndexBuffer(_ib);
		deferred_context_->DrawIndexed(_ib.GetIndexCount(), 0, 0);
	}

	//! @brief インスタンス描画処理
	void dx3d::GraphicsEngine::RenderInstanced(VertexBuffer& _vb, IndexBuffer& _ib, VertexBuffer& _instanceVB, uint32_t _instanceCount, uint32_t _startInstance, PipelineKey _key)
	{
		auto pso = pipeline_cache_->GetOrCreate(_key);
		deferred_context_->SetGraphicsPipelineState(*pso);

		if (_key.GetVS() != VertexShaderKind::ShadowMap) {
			deferred_context_->SetViewportSize(swap_chain_->GetSize());
		}
		deferred_context_->SetVertexBuffers(_vb, _instanceVB);
		deferred_context_->SetIndexBuffer(_ib);
		deferred_context_->DrawIndexedInstanced(_ib.GetIndexCount(), _instanceCount, 0, 0, _startInstance);
	}

	//! @brief 描画終了処理
	void dx3d::GraphicsEngine::EndFrame()
	{
		auto& context = *deferred_context_;
		auto& device = *graphics_device_;
		// コマンドリスト実行
		device.ExecuteCommandList(context);

		// 画面に表示
		swap_chain_->Present();
	}

} // namespace dx3d