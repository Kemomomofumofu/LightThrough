#pragma once
/**
 * @file DeviceContext.h
 * @brief デバイスコンテキスト
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DirectXMath.h>
#include <DX3D/Graphics/GraphicsResource.h>


/**
 * @brief デバイスコンテキスト
 *
 * デバイスコンテキストの仕事をまとめたクラス
 */
namespace dx3d {
	class DeviceContext final : public GraphicsResource
	{
	public:
		explicit DeviceContext(const GraphicsResourceDesc& _gDesc);

		Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetDeviceContext() const noexcept;

		void ClearAndSetBackBuffer(const SwapChain& _swapChain, const DirectX::XMFLOAT4& _color);
		void SetGraphicsPipelineState(const GraphicsPipelineState& _pipepline);
		void SetRasterizerState(const RasterizerState& _rasterizer);

		void SetVertexBuffer(const VertexBuffer& _buffer);
		void SetVertexBuffers(uint32_t _startSlot, const VertexBuffer* const* _buffers, uint32_t _count);	// 複数用
		void SetVertexBuffers(const VertexBuffer& _vb0, const VertexBuffer& _vb1);
		void SetInputLayout(const InputLayout& _layout);
		void SetIndexBuffer(const IndexBuffer& _buffer);
		void VSSetConstantBuffer(uint32_t _slot, const ConstantBuffer& _buffer);
		void PSSetConstantBuffer(uint32_t _slot, const ConstantBuffer& _buffer);
		void PSSetShaderResource(uint32_t _slot, ID3D11ShaderResourceView* _srv);

		void SetConstantBuffer(uint32_t _slot, const ConstantBuffer& _buffer);
		void SetViewportSize(const Rect& _size);

		void DrawTriangleList(uint32_t _vertexCount, uint32_t _startVertexLocation);
		void DrawIndexed(uint32_t _indexCount, uint32_t _startIndex, uint32_t _baseVertex);
		void DrawIndexedInstanced(uint32_t _indexCount, uint32_t _instanceCount, uint32_t _startIndex, uint32_t _baseVertex, uint32_t _startInstance);
	private:
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_{};

		friend class GraphicsDevice;
	};
}
