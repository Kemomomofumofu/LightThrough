#pragma once
/**
 * @file DeviceContext.h
 * @brief デバイスコンテキスト
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include<DX3D/Graphics/GraphicsResource.h>
#include<DX3D/Math/Vec4.h>


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

		void ClearAndSetBackBuffer(const SwapChain& _swapChain, const Vec4& _color);
		void SetGraphicsPipelineState(const GraphicsPipelineState& _pipepline);
		void SetRasterizerState(const RasterizerState& _rasterizer);
		void SetVertexBuffer(const VertexBuffer& _buffer);
		void SetIndexBuffer(const IndexBuffer& _buffer);
		void SetViewportSize(const Rect& _size);
		void DrawTriangleList(ui32 _vertexCount, ui32 _startVertexLocation);
		void DrawIndexed(ui32 _indexCount, ui32 _startIndex, ui32 _baseVertex);
	private:
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_{};

		friend class GraphicsDevice;
	};
}
