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

		// @brief ラスタライザーステートの生成
		rasterizer_ = device.CreateRasterizerState({
			.fillMode = FillMode::Solid,
			.cullMode = CullMode::Back,
			});

		// @brief パイプラインキャッシュの生成
		pipeline_cache_ = device.CreatePipelineCache({});

		// @brief メッシュレジストリの生成
		mesh_registry_ = std::make_unique<MeshRegistry>();

		// @briefMeshを事前に生成しておく
		dx3d::PrimitiveFactory::CreateCube(device, *mesh_registry_);
		dx3d::PrimitiveFactory::CreateSphere(device, *mesh_registry_);
		//line_mesh_ = dx3d::PrimitiveFactory::CreateLine(engine_->GetGraphicsDevice(), {0,0,0}, {1,0,0});


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

	////! @brief デプスクリア
	//void GraphicsEngine::ClearDepth(ID3D11DepthStencilView* _dsv, float _depth, UINT8 _stencil)
	//{
	//	auto context = device_context_->GetDeviceContext();
	//	UINT clearFlags = D3D11_CLEAR_DEPTH;
	//	clearFlags |= D3D11_CLEAR_STENCIL;
	//	context->ClearDepthStencilView(_dsv, clearFlags, _depth, _stencil);
	//}
	//! @brief ビューポート設定
	//void GraphicsEngine::SetViewport(uint32_t _width, uint32_t _height)
	//{
	//	device_context_->SetViewportSize(Rect{ 0, 0, static_cast<int32_t>(_width), static_cast<int32_t>(_height) });
	//}
	////! @brief ラスタライザバイアス設定
	//void GraphicsEngine::SetRasterizerBias(int _depthBias, float _slopeScaleBias, float _depthBiasClamp)
	//{
	//	// todo: RasterizerStateDescにバイアスの項目がないので一時的に直接設定する
	//	D3D11_RASTERIZER_DESC rb{};
	//	rb.FillMode = D3D11_FILL_SOLID;
	//	rb.CullMode = D3D11_CULL_BACK;
	//	rb.FrontCounterClockwise = TRUE;
	//	rb.DepthClipEnable = TRUE;
	//	rb.ScissorEnable = FALSE;
	//	rb.MultisampleEnable = FALSE;
	//	rb.AntialiasedLineEnable = FALSE;
	//	rb.DepthBias = _depthBias;
	//	rb.SlopeScaledDepthBias = _slopeScaleBias;
	//	rb.DepthBiasClamp = _depthBiasClamp;

	//	Microsoft::WRL::ComPtr<ID3D11RasterizerState> biasState;
	//	auto d3d = graphics_device_->GetD3DDevice();
	//	if (SUCCEEDED(d3d->CreateRasterizerState(&rb, &biasState)))
	//	{
	//		device_context_->GetDeviceContext()->RSSetState(biasState.Get());
	//	}
	//	// フォールバック
	//	else {
	//		device_context_->SetRasterizerState(*rasterizer_);	// 元に戻す
	//	}
	//}



	//! @brief 描画開始処理
	void GraphicsEngine::BeginFrame()
	{
		auto& context = *device_context_;
		context.ClearAndSetBackBuffer(*swap_chain_, { 0.27f, 0.39f, 0.55f, 1.0f });	// 初期色でクリア
	}


	//! @brief 描画処理
	void GraphicsEngine::Render(VertexBuffer& _vb, IndexBuffer& _ib)
	{
		PipelineKey key{
			VertexShaderKind::Default,
			PixelShaderKind::Default,
		};
		auto pso = pipeline_cache_->GetOrCreate(key);
		device_context_->SetGraphicsPipelineState(*pso);
		device_context_->SetRasterizerState(*rasterizer_);	// [ToDo] pso で設定できるようにしたいね
		device_context_->SetViewportSize(swap_chain_->GetSize());

		device_context_->SetVertexBuffer(_vb);
		device_context_->SetIndexBuffer(_ib);
		device_context_->DrawIndexed(_ib.GetIndexCount(), 0, 0);
	}

	//! @brief インスタンス描画処理
	void dx3d::GraphicsEngine::RenderInstanced(VertexBuffer& _vb, IndexBuffer& _ib, VertexBuffer& _instanceVB, uint32_t _instanceCount, uint32_t _startInstance)
	{
		PipelineKey key{
			VertexShaderKind::Instanced,
			PixelShaderKind::Default,
		};
		PromoteInstancing(key, _instanceCount);	// インスタンス数に応じてパイプラインキーを更新
		auto pso = pipeline_cache_->GetOrCreate(key);
		device_context_->SetGraphicsPipelineState(*pso);
		device_context_->SetRasterizerState(*rasterizer_);	// [ToDo] pso で設定できるようにしたいね
		device_context_->SetViewportSize(swap_chain_->GetSize());

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