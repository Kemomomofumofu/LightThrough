/**
 * @file DeviceContext.cpp
 * @brief デバイスコンテキスト
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/GraphicsPipelineState.h>
#include <DX3D/Graphics/RasterizerState.h>
#include <DX3D/Graphics/Buffers/Vertex.h>
#include <DX3D/Graphics/Buffers/VertexBuffer.h>
#include <DX3D/Graphics/Buffers/IndexBuffer.h>
#include <DX3D/Graphics/Buffers/ConstantBuffer.h>


namespace dx3d {
	/**
	 * @brief コンストラクタ
	 * @param _gDesc
	 */
	DeviceContext::DeviceContext(const GraphicsResourceDesc& _gDesc)
		: GraphicsResource(_gDesc)
	{
		DX3DGraphicsLogThrowOnFail(device_.CreateDeferredContext(0, &context_), "CreateDeferredContext を 失敗しました");

		ID3D11DeviceContext* imm = nullptr;
		graphics_device_->GetD3DDevice()->GetImmediateContext(&imm);
	}

	/**
	 * @brief ゲッター
	 * @return デバイスコンテキストのポインタ
	 */

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

	/**
	 * @brief ラスタライザーステートをセットする
	 * @param _rasterizer
	 */
	void DeviceContext::SetRasterizerState(const RasterizerState& _rasterizer)
	{
		context_->RSSetState(_rasterizer.rast_state_.Get());
	}

	/**
	 * @brief 頂点バッファをセットする
	 * @param _buffer 頂点バッファ
	 */
	void DeviceContext::SetVertexBuffer(const VertexBuffer& _buffer)
	{
		auto stride = _buffer.vertex_size_;
		auto buf = _buffer.buffer_.Get();
		auto offset = 0u;
		context_->IASetVertexBuffers(0, 1, &buf, &stride, &offset);
	}

	/**
	 * @brief 頂点バッファをセットする（複数用）
	 * @param _startSlot 開始スロット
	 * @param _buffers 頂点バッファの配列
	 * @param _count 頂点バッファの数
	 */
	void DeviceContext::SetVertexBuffers(ui32 _startSlot, const VertexBuffer* const* _buffers, ui32 _count)
	{
		UINT strides[2] = { sizeof(Vertex), sizeof(InstanceData) };
		UINT offsets[2] = { 0, 0 };
		ID3D11Buffer* bufs[2] = { _buffers[0]->GetBuffer(), _buffers[1]->GetBuffer()};
		context_->IASetVertexBuffers(_startSlot, _count, bufs, strides, offsets);
	}

	/**
	 * @brief 頂点バッファをセットする（2つ用）
	 * @param _vb0 頂点バッファ0
	 * @param _vb1 頂点バッファ1
	 */
	void DeviceContext::SetVertexBuffers(const VertexBuffer& _vb0, const VertexBuffer& _vb1)
	{
		const VertexBuffer* arr[2] = { &_vb0, &_vb1 };
		SetVertexBuffers(0, arr, 2);
	}

	/**
	 * @brief インデックスバッファをセットする
	 * @param _buffer インデックスバッファ
	 */
	void DeviceContext::SetIndexBuffer(const IndexBuffer& _buffer)
	{
		context_->IASetIndexBuffer(_buffer.GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
	}

	/**
	 * @brief 頂点シェーダーの定数バッファをセットする
	 * @param _slot スロット
	 * @param _buffer 定数バッファ
	 */
	void DeviceContext::VSSetConstantBuffer(ui32 _slot, const ConstantBuffer& _buffer)
	{
		ID3D11Buffer* buffer = _buffer.GetBuffer();
		context_->VSSetConstantBuffers(_slot, 1, &buffer);
	}

	/**
	 * @brief ピクセルシェーダーの定数バッファをセットする
	 * @param _slot スロット
	 * @param _buffer 定数バッファ
	 */
	void DeviceContext::PSSetConstantBuffer(ui32 _slot, const ConstantBuffer& _buffer)
	{
		ID3D11Buffer* buffer = _buffer.GetBuffer();
		context_->PSSetConstantBuffers(_slot, 1, &buffer);
	}

	/**
	 * @brief 頂点シェーダーとピクセルシェーダーの定数バッファをセットする
	 * @param _slot スロット
	 * @param _buffer 定数バッファ
	 */
	void DeviceContext::SetConstantBuffer(ui32 _slot, const ConstantBuffer& _buffer)
	{
		VSSetConstantBuffer(_slot, _buffer);
		PSSetConstantBuffer(_slot, _buffer);
	}

	/**
	 * @brief ビューポートサイズを設定する
	 * @param _size サイズ
	 */
	void dx3d::DeviceContext::SetViewportSize(const Rect& _size)
	{
		D3D11_VIEWPORT vp{};
		vp.Width = static_cast<f32>(_size.width);
		vp.Height = static_cast<f32>(_size.height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		context_->RSSetViewports(1, &vp);
	}

	/**
	 * @brief 三角形リストを描画する
	 * @param _vertexCount 頂点数
	 * @param _startVertexLocation 開始頂点位置
	 */
	void dx3d::DeviceContext::DrawTriangleList(ui32 _vertexCount, ui32 _startVertexLocation)
	{
		context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context_->Draw(_vertexCount, _startVertexLocation);
	}

	/**
	 * @brief インデックス付きプリミティブを描画する
	 * @param _indexCount	インデックス数
	 * @param _startIndex	開始インデックス位置
	 * @param _baseVertex	開始頂点位置
	 */
	void dx3d::DeviceContext::DrawIndexed(ui32 _indexCount, ui32 _startIndex, ui32 _baseVertex)
	{
		context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context_->DrawIndexed(_indexCount, _startIndex, _baseVertex);
	}

	/**
	 * @brief インスタンス化されたインデックス付きプリミティブを描画する
	 * @param _indexCount		インデックス数
	 * @param _instanceCount	インスタンス数
	 * @param _startIndex		開始インデックス位置
	 * @param _baseVertex		開始頂点位置
	 * @param _startInstance	開始インスタンス位置
	 */
	void DeviceContext::DrawIndexedInstanced(ui32 _indexCount, ui32 _instanceCount, ui32 _startIndex, ui32 _baseVertex, ui32 _startInstance)
	{
		context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context_->DrawIndexedInstanced(_indexCount, _instanceCount, _startIndex, _baseVertex, _startInstance);
	}

}