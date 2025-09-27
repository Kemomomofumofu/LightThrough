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
#include <DX3D/Graphics/PipelineCache.h>
#include <DX3D/Graphics/PipelineKey.h>


/**
 * @brief グラフィックエンジンクラス
 *
 * なんか描画する感じ？
 */
namespace dx3d {
	class GraphicsEngine final : public Base {
	public:
		explicit GraphicsEngine(const GraphicsEngineDesc& _desc);
		virtual ~GraphicsEngine() override;

		GraphicsDevice& GetGraphicsDevice() noexcept;
		DeviceContext& GetDeviceContext() noexcept;
		void SetSwapChain(SwapChain& _swapChain);

		void BeginFrame();
		void Render(VertexBuffer& _vb, IndexBuffer& _ib);
		void RenderInstanced(VertexBuffer& _vb, IndexBuffer& _ib, VertexBuffer& _instanceVB, uint32_t _instanceCount, uint32_t _startInstance = 0);
		void EndFrame();

	private:
		std::shared_ptr<GraphicsDevice> graphics_device_{};
		DeviceContextPtr device_context_{};
		GraphicsPipelineStatePtr pipeline_{};
		RasterizerStatePtr rasterizer_{};
		InputLayoutPtr vs_layout_{};
		std::unique_ptr<PipelineCache> pipeline_cache_{};
		SwapChain* swap_chain_{};
	};
}
