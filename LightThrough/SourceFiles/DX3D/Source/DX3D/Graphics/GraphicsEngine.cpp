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

#include <Debug/Debug.h>

namespace dx3d {
	//! @brief コンストラクタ
	GraphicsEngine::GraphicsEngine(const GraphicsEngineDesc& _desc)
		: Base(_desc.base)
	{
		graphics_device_ = std::make_shared<GraphicsDevice>(GraphicsDeviceDesc{ logger_ });
		
		auto& device = *graphics_device_;
		device_context_ = device.CreateDeviceContext();

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
	DeviceContext& GraphicsEngine::GetDeviceContext() noexcept
	{
		return *device_context_;
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
		auto& context = *device_context_;
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
		device_context_->SetGraphicsPipelineState(*pso);
		device_context_->SetViewportSize(swap_chain_->GetSize());
		device_context_->SetVertexBuffer(_vb);
		device_context_->SetIndexBuffer(_ib);
		device_context_->DrawIndexed(_ib.GetIndexCount(), 0, 0);
	}

	//! @brief インスタンス描画処理
	void dx3d::GraphicsEngine::RenderInstanced(VertexBuffer& _vb, IndexBuffer& _ib, VertexBuffer& _instanceVB, uint32_t _instanceCount, uint32_t _startInstance, PipelineKey _key)
	{
		auto pso = pipeline_cache_->GetOrCreate(_key);
		device_context_->SetGraphicsPipelineState(*pso);

		if (!(_key.GetFlags() & PipelineFlags::ShadowPass)) {
			device_context_->SetViewportSize(swap_chain_->GetSize());
		}
		device_context_->SetVertexBuffers(_vb, _instanceVB);
		device_context_->SetIndexBuffer(_ib);
		device_context_->DrawIndexedInstanced(_ib.GetIndexCount(), _instanceCount, 0, 0, _startInstance);
	}

	//! @brief 描画終了処理
	void dx3d::GraphicsEngine::EndFrame()
	{
		auto& context = *device_context_;
		auto& device = *graphics_device_;
		// コマンドリスト実行
		device.ExecuteCommandList(context);

		// 画面に表示
		swap_chain_->Present();
	}

} // namespace dx3d