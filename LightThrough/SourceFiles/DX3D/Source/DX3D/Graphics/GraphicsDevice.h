#pragma once

/**
 * @file GraphicsDevice.h
 * @brief グラフィックデバイス
 * @author Arima Keita
 * @date 2025-07-14
 */

 /*---------- インクルード ----------*/
#include <memory>
#include <vector>
#include <d3d11.h>
#include <wrl.h>
#include <DX3D/Core/common.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Graphics/Buffers/InstanceData.h>
#include <DX3D/Graphics/Inputlayout.h>
#include <DX3D/Graphics/ShaderCache.h>
#include <DX3D/Graphics/PipelineCache.h>
#include <DX3D/Graphics/ShaderBinary.h>
#include <DX3D/Graphics/Buffers/StructuredBuffers.h>

namespace dx3d {
	class GraphicsDevice final : public Base, public std::enable_shared_from_this<GraphicsDevice> {
	public:
		explicit GraphicsDevice(const GraphicsDeviceDesc& _desc);
		virtual ~GraphicsDevice() override;

		SwapChainPtr CreateSwapChain(const SwapChainDesc& _desc);
		DeviceContextPtr CreateDeviceContext();
		ShaderBinaryPtr CompileShader(const ShaderBinary::ShaderCompileDesc& _desc) const;
		GraphicsPipelineStatePtr CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& _desc) const;
		VertexShaderSignaturePtr CreateVertexShaderSignature(const VertexShaderSignatureDesc& _desc) const;
		InputLayoutPtr CreateInputLayout(const InputLayoutDesc& _desc) const;
		VertexBufferPtr CreateVertexBuffer(const VertexBufferDesc& _desc);
		IndexBufferPtr CreateIndexBuffer(const IndexBufferDesc& _desc);
		ConstantBufferPtr CreateConstantBuffer(const ConstantBufferDesc& _desc);
		StructuredBufferPtr CreateStructuredBuffer(const StructuredBufferDesc& _desc);
		RWStructuredBufferPtr CreateRWStructuredBuffer(const RWStructuredBufferDesc& _desc);
		StagingBufferPtr CreateStagingBuffer(const StagingBufferDesc& _desc);
		auto CreateInstanceBuffer(const std::vector<InstanceDataMain>& _data);
		std::unique_ptr<ShaderCache> CreateShaderCache(const ShaderCache::ShaderCacheDesc& _desc);
		std::unique_ptr<PipelineCache> CreatePipelineCache(const PipelineCache::PipelineCacheDesc& _desc);

		HRESULT CreateTexture2D(const D3D11_TEXTURE2D_DESC* _desc, const D3D11_SUBRESOURCE_DATA* _initialData, ID3D11Texture2D** _texture)const noexcept;
		HRESULT CreateDepthStencilView(ID3D11Resource* _resource, const D3D11_DEPTH_STENCIL_VIEW_DESC* _desc, ID3D11DepthStencilView** _dvs) const noexcept;
		HRESULT CreateShaderResourceView(ID3D11Resource* _resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* _desc, ID3D11ShaderResourceView** _srv) const noexcept;

		void ExecuteCommandList(DeviceContext& _context);

		Microsoft::WRL::ComPtr<ID3D11Device> GetD3DDevice() const noexcept { return d3d_device_; }
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetD3DContext() const noexcept { return d3d_context_; }
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