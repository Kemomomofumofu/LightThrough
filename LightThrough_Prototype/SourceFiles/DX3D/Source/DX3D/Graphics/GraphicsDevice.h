#pragma once

/**
 * @file GraphicsDevice.h
 * @brief グラフィックデバイス
 * @author Arima Keita
 * @date 2025-07-14
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Core/common.h>
#include <DX3D/Core/Base.h>
#include <d3d11.h>
#include <wrl.h>

namespace dx3d {
	class GraphicsDevice final : public Base, public std::enable_shared_from_this<GraphicsDevice> {
	public:
		explicit GraphicsDevice(const GraphicsDeviceDesc& _desc);
		virtual ~GraphicsDevice() override;

		SwapChainPtr CreateSwapChain(const SwapChainDesc& _desc);
		DeviceContextPtr CreateDeviceContext();
		ShaderBinaryPtr CompileShader(const ShaderCompileDesc& _desc);
		GraphicsPipelineStatePtr CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& _desc);
		VertexBufferPtr CreateVertexBuffer(const VertexBufferDesc& _desc);

		void ExecuteCommandList(DeviceContext& _context);

	private:
		GraphicsResourceDesc GetGraphicsResourceDesc() const noexcept;

	private:
		Microsoft::WRL::ComPtr<ID3D11Device> d3d_device_{};					// デバイス
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d_context_{};	// デバイスコンテキスト
		Microsoft::WRL::ComPtr<IDXGIDevice> dxgi_device_{};		// GPUとWindowsの橋
		Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter_{};		// GPUの情報を持っている
		Microsoft::WRL::ComPtr<IDXGIFactory> dxgi_factory_{};		// スワップチェインを作ったり
	};
}