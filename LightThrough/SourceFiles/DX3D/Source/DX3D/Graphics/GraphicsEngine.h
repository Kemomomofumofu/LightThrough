#pragma once
/**
 * @file GraphicsEngine.h
 * @brief グラフィックエンジンクラス
 * @author Arima Keita
 * @date 2025-07-11
 */

 /*---------- インクルード ----------*/
#include <DX3D/Core/Core.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Graphics/ShaderCache.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/PipelineCache.h>
#include <DX3D/Graphics/PipelineKey.h>
#include <DX3D/Graphics/Meshes/MeshRegistry.h>


/**
 * @brief グラフィックエンジンクラス
 *
 * なんか描画する感じ？
 */
namespace dx3d {
	class TextureRegistry;

	struct GraphicsEngineDesc {
		BaseDesc base;
	};

	class GraphicsEngine final : public Base {
	public:

		explicit GraphicsEngine(const GraphicsEngineDesc& _desc);
		virtual ~GraphicsEngine() override;
		
		GraphicsDevice& GetGraphicsDevice() noexcept;
		DeviceContext& GetDeferredContext() noexcept;
		ID3D11DeviceContext* GetImmediateContext() noexcept;
		
		//! @brief メッシュレジストリ取得
		MeshRegistry& GetMeshRegistry() noexcept;
		//! @brief シェーダーキャッシュ取得
		ShaderCache& GetShaderCache() noexcept { return *shader_cache_; };
		const Rect& GetScreenSize() { return swap_chain_->GetSize(); }

		void SetSwapChain(SwapChain& _swapChain);
		

		void BeginFrame();
		void Render(VertexBuffer& _vb, IndexBuffer& _ib, PipelineKey _key = { VertexShaderKind::Default,
			PixelShaderKind::Default });
		void RenderInstanced(VertexBuffer& _vb, IndexBuffer& _ib, VertexBuffer& _instanceVB, uint32_t _instanceCount, uint32_t _startInstance = 0, PipelineKey _key = {VertexShaderKind::Instanced, PixelShaderKind::Default});
		void EndFrame();

	private:
		std::shared_ptr<GraphicsDevice> graphics_device_{};
		DeviceContextPtr deferred_context_{};
		GraphicsPipelineStatePtr pipeline_{};
		InputLayoutPtr vs_layout_{};
		std::unique_ptr<ShaderCache> shader_cache_{};
		std::unique_ptr<PipelineCache> pipeline_cache_{};
		SwapChain* swap_chain_{};
		std::unique_ptr<MeshRegistry> mesh_registry_{};
		std::unique_ptr<TextureRegistry> texture_registry_{};
	};
}
