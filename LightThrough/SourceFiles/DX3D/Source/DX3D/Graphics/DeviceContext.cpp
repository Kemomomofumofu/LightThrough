/**
 * @file DeviceContext.cpp
 * @brief デバイスコンテキスト
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/GraphicsPipelineState.h>
#include <DX3D/Graphics/RasterizerState.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/IndexBuffer.h>
#include <DX3D/Graphics/ConstantBuffer.h>


namespace dx3d {
	/**
	 * @brief コンストラクタ
	 * @param _gDesc
	 */
	DeviceContext::DeviceContext(const GraphicsResourceDesc& _gDesc)
		: GraphicsResource(_gDesc)
	{
		DX3DGraphicsLogThrowOnFail(device_.CreateDeferredContext(0, &context_), "CreateDeferredContext を 失敗しました");
	}

	Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext::GetDeviceContext() const noexcept
	{
		return context_;
	}


	/**
	 * @brief バックバッファをクリアしたりセットしたり
	 * @param _swapChain スワップチェイン
	 * @param _color 初期化色
	 */
	void DeviceContext::ClearAndSetBackBuffer(const SwapChain& _swapChain, const Vec4& _color)
	{
		f32 fColor[] = { _color.x, _color.y, _color.z, _color.w };
		auto rtv = _swapChain.rtv_.Get();
		context_->ClearRenderTargetView(rtv, fColor);
		context_->OMSetRenderTargets(1, &rtv, nullptr);
	}


	/**
	 * @brief InputLayoutやシェーダーをセットする
	 * @param _pipeline
	 */
	void DeviceContext::SetGraphicsPipelineState(const GraphicsPipelineState& _pipeline)
	{
		context_->IASetInputLayout(_pipeline.layout_.Get());
		context_->VSSetShader(_pipeline.vs_.Get(), nullptr, 0);
		context_->PSSetShader(_pipeline.ps_.Get(), nullptr, 0);
	}

	void DeviceContext::SetRasterizerState(const RasterizerState& _rasterizer)
	{
		context_->RSSetState(_rasterizer.rast_state_.Get());
	}

	void DeviceContext::SetVertexBuffer(const VertexBuffer& _buffer)
	{
		auto stride = _buffer.vertex_size_;
		auto buf = _buffer.buffer_.Get();
		auto offset = 0u;
		context_->IASetVertexBuffers(0, 1, &buf, &stride, &offset);
	}

	void DeviceContext::SetIndexBuffer(const IndexBuffer& _buffer)
	{
		context_->IASetIndexBuffer(_buffer.GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
	}

	void DeviceContext::VSSetConstantBuffer(ui32 _slot, const ConstantBuffer& _buffer)
	{
		ID3D11Buffer* buffer = _buffer.GetBuffer();
		context_->VSSetConstantBuffers(_slot, 1, &buffer);
	}

	void DeviceContext::PSSetConstantBuffer(ui32 _slot, const ConstantBuffer& _buffer)
	{
		ID3D11Buffer* buffer = _buffer.GetBuffer();
		context_->PSSetConstantBuffers(_slot, 1, &buffer);
	}

	void DeviceContext::SetConstantBuffer(ui32 _slot, const ConstantBuffer& _buffer)
	{
		VSSetConstantBuffer(_slot, _buffer);
		PSSetConstantBuffer(_slot, _buffer);
	}

	void dx3d::DeviceContext::SetViewportSize(const Rect& _size)
	{
		D3D11_VIEWPORT vp{};
		vp.Width = static_cast<f32>(_size.width);
		vp.Height = static_cast<f32>(_size.height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		context_->RSSetViewports(1, &vp);
	}

	void dx3d::DeviceContext::DrawTriangleList(ui32 _vertexCount, ui32 _startVertexLocation)
	{
		context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context_->Draw(_vertexCount, _startVertexLocation);
	}

	void dx3d::DeviceContext::DrawIndexed(ui32 _indexCount, ui32 _startIndex, ui32 _baseVertex)
	{
		context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context_->DrawIndexed(_indexCount, _startIndex, _baseVertex);
	}

}