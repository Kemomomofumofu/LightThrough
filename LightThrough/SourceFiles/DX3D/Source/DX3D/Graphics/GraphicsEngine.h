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
#include <DX3D/Math/Vec3.h>
#include <DX3D/Math/Vec4.h>


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

		void Render(SwapChain& _swapChain);

	private:
		struct Vertex {
			Vec3 position;
			Vec4 color;
		};

	private:
		std::shared_ptr<GraphicsDevice> graphics_device_{};
		DeviceContextPtr device_context_{};
		GraphicsPipelineStatePtr pipeline_{};
		VertexBufferPtr vb_{};
	};
}
