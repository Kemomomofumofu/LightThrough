/**
 * @file GraphicsEngine.cpp
 * @brief グラフィックスエンジンクラスの定義
 * @author Arima Keita
 * @date 2025-07-11
 */

 /*---------- インクルード ----------*/
#include <fstream>
#include <algorithm>

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

	GraphicsEngine::GraphicsEngine(const GraphicsEngineDesc& _desc)
		: Base(_desc.base)
	{
		graphics_device_ = std::make_shared<GraphicsDevice>(GraphicsDeviceDesc{ logger_ });
		deferred_context_ = graphics_device_->CreateDeviceContext();

		shader_cache_ = graphics_device_->CreateShaderCache({});
		pipeline_cache_ = graphics_device_->CreatePipelineCache({ *shader_cache_ });

		mesh_registry_ = std::make_unique<MeshRegistry>();
		PrimitiveFactory::CreateCube(*graphics_device_, *mesh_registry_);
		PrimitiveFactory::CreateSphere(*graphics_device_, *mesh_registry_);
		PrimitiveFactory::CreateQuad(*graphics_device_, *mesh_registry_);

		ID3D11Device* devicePtr = graphics_device_->GetD3DDevice().Get();
		texture_registry_ = std::make_unique<TextureRegistry>(devicePtr);
		texture_registry_->Load("hogehoge.png");
		texture_registry_->Load("TitleLogo.png");

		InitPostProcessChain();
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
	//! @brief 即時コンテキスト取得
	ID3D11DeviceContext* GraphicsEngine::GetImmediateContext() noexcept
	{
		return graphics_device_->GetImmediateContext();
	}
	//! @brief スワップチェイン設定
	void GraphicsEngine::SetSwapChain(SwapChain& _swapChain)
	{
		swap_chain_ = &_swapChain;
		EnsureFrameResources();
	}
	//! @brief メッシュレジストリ取得
	MeshRegistry& GraphicsEngine::GetMeshRegistry() noexcept
	{
		return *mesh_registry_;
	}

	//! @brief 描画開始処理
	void GraphicsEngine::BeginFrame()
	{
		EnsureFrameResources();

		auto& context = *deferred_context_;
		DirectX::XMFLOAT4 clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
#ifdef _DEBUG || DEBUG
		clearColor = { 0.2, 0.3f, 0.5f, 1.0f };
#endif
		context.ClearAndSetRenderTarget(scene_color_.rtv.Get(), scene_depth_dsv_.Get(), clearColor); // リソースのクリア
		context.SetViewportSize(swap_chain_->GetSize());	}


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

	//! @brief 遅延コンテキストでのインスタンス描画
	void GraphicsEngine::RenderInstanced(VertexBuffer& _vb, IndexBuffer& _ib, VertexBuffer& _instanceVB, uint32_t _instanceCount, uint32_t _startInstance, PipelineKey _key)
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
	//! @brief 即時コンテキストでのインスタンス描画
	void GraphicsEngine::RenderInstancedOnImmediate(VertexBuffer& _vb, IndexBuffer& _ib, VertexBuffer& _instanceVB, uint32_t _instanceCount, uint32_t _startInstance, PipelineKey _key)
	{
		auto immediateContext = graphics_device_->GetImmediateContext();

		// 即時コンテキストにパイプラインステートをセット
		auto pso = pipeline_cache_->GetOrCreate(_key);
		pso->Apply(immediateContext);

		ID3D11Buffer* vbs[2] = { _vb.GetBuffer(), _instanceVB.GetBuffer() };
		UINT strides[2] = { _vb.GetVertexSize(), _instanceVB.GetVertexSize() };
		UINT offsets[2] = { 0, 0 };
		immediateContext->IASetVertexBuffers(0, 2, vbs, strides, offsets);

		immediateContext->IASetIndexBuffer(_ib.GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
		// 描画
		immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		immediateContext->DrawIndexedInstanced(_ib.GetIndexCount(), _instanceCount, 0, 0, _startInstance);
	}

	//! @brief 描画終了処理
	void GraphicsEngine::EndFrame()
	{
		RunPostProcessChain();
		ResolveSceneToBackBuffer();

		auto& context = *deferred_context_;
		auto& device = *graphics_device_;
		device.ExecuteCommandList(context);
		swap_chain_->Present();
	}

	void GraphicsEngine::InitPostProcessChain()
	{
		post_chain_.clear();
		post_chain_.push_back(PostPassKind::Outline);
		post_chain_.push_back(PostPassKind::Vhs);
	}

	void GraphicsEngine::RunPostProcessChain()
	{
		if (post_chain_.empty()) { return; }

		const OffscreenTarget* current = &scene_color_;
		bool useA = true;

		for (const auto pass : post_chain_) {
			OffscreenTarget& dst = useA ? post_a_ : post_b_;
			ExecutePostPass(pass, *current, dst);
			current = &dst;
			useA = !useA;
		}

		// 最終結果を SceneColor に戻しておく（後段の Resolve がそのまま使える）
		if (current != &scene_color_) {
			auto* ctx = deferred_context_->GetDeferredContext().Get();
			ctx->CopyResource(scene_color_.texture.Get(), current->texture.Get());
		}
	}

	void GraphicsEngine::ExecutePostPass(PostPassKind _kind, const OffscreenTarget& _src, OffscreenTarget& _dst)
	{
		auto& context = *deferred_context_;

		PixelShaderKind psKind = PixelShaderKind::PostVhs;
		switch (_kind) {
		case PostPassKind::Outline:
			psKind = PixelShaderKind::PostOutline;
			break;
		case PostPassKind::Vhs:
			psKind = PixelShaderKind::PostVhs;
			break;
		}

		const auto key = BuildPipelineKey(
			VertexShaderKind::Fullscreen,
			psKind,
			BlendMode::Opaque,
			DepthMode::Disable,
			RasterMode::SolidBack
		);

		auto pso = pipeline_cache_->GetOrCreate(key);
		context.SetGraphicsPipelineState(*pso);
		context.SetViewportSize(swap_chain_->GetSize());

		ID3D11RenderTargetView* rtv = _dst.rtv.Get();
		context.SetRenderTargets(1, &rtv, nullptr);

		DirectX::XMFLOAT4 clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		context.ClearRenderTarget(rtv, clearColor);

		ID3D11ShaderResourceView* srv = _src.srv.Get();
		context.PSSetShaderResources(0, 1, &srv);

		ID3D11SamplerState* sampler = post_linear_sampler_.Get();
		context.PSSetSamplers(0, 1, &sampler);

		context.DrawTriangleList(3, 0);

		ID3D11ShaderResourceView* nullSrv[1] = { nullptr };
		context.PSSetShaderResources(0, 1, nullSrv);
	}

	void GraphicsEngine::ExecutePassthrough(const OffscreenTarget& _src, OffscreenTarget& _dst)
	{
		auto* ctx = deferred_context_->GetDeferredContext().Get();

		// SRV/RTV競合の予防
		ID3D11ShaderResourceView* nullSrvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
		ctx->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSrvs);

		ID3D11RenderTargetView* nullRtvs[1]{};
		ctx->OMSetRenderTargets(1, nullRtvs, nullptr);

		ctx->CopyResource(_dst.texture.Get(), _src.texture.Get());
	}

	void GraphicsEngine::ResolveSceneToBackBuffer()
	{
		if (!swap_chain_ || !scene_color_.texture) { return; }

		auto* ctx = deferred_context_->GetDeferredContext().Get();

		ID3D11ShaderResourceView* nullSrvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
		ctx->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSrvs);

		ID3D11RenderTargetView* nullRtvs[1]{};
		ctx->OMSetRenderTargets(1, nullRtvs, nullptr);

		auto backBuffer = swap_chain_->GetBackBufferTexture();
		if (!backBuffer) { return; }

		ctx->CopyResource(backBuffer.Get(), scene_color_.texture.Get());
	}

} // namespace dx3d