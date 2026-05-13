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
#include <DX3D/Graphics/Buffers/Vertex.h>
#include <DX3D/Graphics/Buffers/VertexBuffer.h>
#include <DX3D/Graphics/Buffers/IndexBuffer.h>
#include <DX3D/Graphics/Buffers/ConstantBuffer.h>
#include <DX3D/Graphics/Buffers/StructuredBuffers.h>


namespace dx3d {
	//! @brief コンストラクタ
	DeviceContext::DeviceContext(const GraphicsResourceDesc& _gDesc)
		: GraphicsResource(_gDesc)
	{
		DX3DGraphicsLogThrowOnFail(device_.CreateDeferredContext(0, &deferred_context_), "CreateDeferredContext を 失敗しました");
	}

	//! @brief デバイスコンテキストを取得
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext::GetDeferredContext() const noexcept
	{
		return deferred_context_;
	}

	//! @brief バックバッファをクリアしてセット
	void DeviceContext::ClearAndSetBackBuffer(const SwapChain& _swapChain, const DirectX::XMFLOAT4& _color)
	{
		float fColor[] = { _color.x, _color.y, _color.z, _color.w };
		auto rtv = _swapChain.rtv_.Get();
		auto dsv = _swapChain.dsv_.Get();
		deferred_context_->ClearRenderTargetView(rtv, fColor);
		deferred_context_->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		deferred_context_->OMSetRenderTargets(1, &rtv, dsv);
	}

	//! @brief レンダーターゲットをクリアしてセット
	void DeviceContext::ClearAndSetRenderTarget(ID3D11RenderTargetView* _rtv, ID3D11DepthStencilView* _dsv, const DirectX::XMFLOAT4& _color)
	{
		float fColor[] = { _color.x, _color.y, _color.z, _color.w };
		deferred_context_->ClearRenderTargetView(_rtv, fColor);
		if (_dsv) {
			deferred_context_->ClearDepthStencilView(_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
		deferred_context_->OMSetRenderTargets(1, &_rtv, _dsv);
	}

	//! @brief レンダーターゲットをセット
	void DeviceContext::SetRenderTargets(uint32_t _numRtvs, ID3D11RenderTargetView* const* _rtvs, ID3D11DepthStencilView* _dsv)
	{
		deferred_context_->OMSetRenderTargets(_numRtvs, _rtvs, _dsv);
	}

	//! @brief レンダーターゲットをクリア
	void DeviceContext::ClearRenderTarget(ID3D11RenderTargetView* _rtv, const DirectX::XMFLOAT4& _color)
	{
		float fColor[] = { _color.x, _color.y, _color.z, _color.w };
		deferred_context_->ClearRenderTargetView(_rtv, fColor);
	}

	//! @brief 深度ステンシルをクリア
	void DeviceContext::ClearDepthStencil(ID3D11DepthStencilView* _dsv, float _depth, uint8_t _stencil)
	{
		if (!_dsv) { return; }

		deferred_context_->ClearDepthStencilView(_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, _depth, _stencil);
	}

	//! @brief グラフィックスパイプラインステートをセット
	void DeviceContext::SetGraphicsPipelineState(const GraphicsPipelineState& _pipeline)
	{
		_pipeline.Apply(deferred_context_.Get());
	}

	//! @brief 頂点バッファをセット
	void DeviceContext::SetVertexBuffer(const VertexBuffer& _buffer)
	{
		auto stride = _buffer.vertex_size_;
		auto buf = _buffer.buffer_.Get();
		auto offset = 0u;
		deferred_context_->IASetVertexBuffers(0, 1, &buf, &stride, &offset);
	}

	//! @brief 複数の頂点バッファをセット
	void DeviceContext::SetVertexBuffers(uint32_t _startSlot, const VertexBuffer* const* _buffers, uint32_t _count)
	{
		ID3D11Buffer* bufs[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT]{};
		UINT strides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT]{};
		UINT offsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT]{};

		for (uint32_t i = 0; i < _count; ++i) {
			bufs[i] = _buffers[i]->GetBuffer();
			strides[i] = _buffers[i]->vertex_size_;
			offsets[i] = 0;
		}
		deferred_context_->IASetVertexBuffers(_startSlot, _count, bufs, strides, offsets);
	}

	//! @brief 2つの頂点バッファをセット
	void DeviceContext::SetVertexBuffers(const VertexBuffer& _vb0, const VertexBuffer& _vb1)
	{
		const VertexBuffer* arr[2] = { &_vb0, &_vb1 };
		SetVertexBuffers(0, arr, 2);
	}

	//! @brief 入力レイアウトをセット
	void DeviceContext::SetInputLayout(const InputLayout& _layout)
	{
		deferred_context_->IASetInputLayout(_layout.Get());
	}

	//! @brief インデックスバッファをセット
	void DeviceContext::SetIndexBuffer(const IndexBuffer& _buffer)
	{
		deferred_context_->IASetIndexBuffer(_buffer.GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
	}

	//! @brief 頂点シェーダに定数バッファをセット
	void DeviceContext::VSSetConstantBuffer(uint32_t _slot, const ConstantBuffer& _buffer)
	{
		ID3D11Buffer* buffer = _buffer.GetBuffer();
		deferred_context_->VSSetConstantBuffers(_slot, 1, &buffer);
	}

	//---------- ピクセルシェーダー関係 ----------//
	//! @brief ピクセルシェーダに定数バッファをセット
	void DeviceContext::PSSetConstantBuffer(uint32_t _slot, const ConstantBuffer& _buffer)
	{
		ID3D11Buffer* buffer = _buffer.GetBuffer();
		deferred_context_->PSSetConstantBuffers(_slot, 1, &buffer);
	}

	//! @brief ピクセルシェーダのシェーダーリソースビューをセットする
	void DeviceContext::PSSetShaderResources(uint32_t _startSlot, uint32_t _numResources, ID3D11ShaderResourceView* const* _ppSrv)
	{
		deferred_context_->PSSetShaderResources(_startSlot, _numResources, _ppSrv);
	}

	//! @brief ピクセルシェーダのサンプラーをセットする
	void DeviceContext::PSSetSamplers(uint32_t _startSlot, uint32_t _numSamplers, ID3D11SamplerState* const* _ppSampler)
	{
		deferred_context_->PSSetSamplers(_startSlot, _numSamplers, _ppSampler);
	}

	//! @brief コンピュートシェーダをセット
	void DeviceContext::CSSetShader(ID3D11ComputeShader* _cs)
	{
		deferred_context_->CSSetShader(_cs, nullptr, 0);
	}

	//! @brief コンピュートシェーダの定数バッファをセット
	void DeviceContext::CSSetConstantBuffer(uint32_t _slot, const ConstantBuffer& _cb)
	{
		ID3D11Buffer* buffer = _cb.GetBuffer();
		deferred_context_->CSSetConstantBuffers(_slot, 1, &buffer);
	}

	//! @brief コンピュートシェーダの定数バッファを複数セット
	void DeviceContext::CSSetConstantBuffers(uint32_t _startSlot, uint32_t _numBuffers, const ConstantBuffer* const* _buffers)
	{
		ID3D11Buffer* bufs[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT]{};
		for (uint32_t i = 0; i < _numBuffers; ++i) {
			bufs[i] = _buffers[i]->GetBuffer();
		}
		deferred_context_->CSSetConstantBuffers(_startSlot, _numBuffers, bufs);
	}

	//! @brief コンピュートシェーダのシェーダーリソースビューをセット
	void DeviceContext::CSSetShaderResources(uint32_t _startSlot, StructuredBuffer& _buffer)
	{
		ID3D11ShaderResourceView* srv = _buffer.GetSRV();
		deferred_context_->CSSetShaderResources(_startSlot, 1, &srv);
	}

	//! @brief コンピュートシェーダのシェーダーリソースビューをセット
	void DeviceContext::CSSetShaderResources(uint32_t _startSlot, ID3D11ShaderResourceView* _srv)
	{
		deferred_context_->CSSetShaderResources(_startSlot, 1, &_srv);
	}

	//! @brief コンピュートシェーダのシェーダーリソースビューをセット
	void DeviceContext::CSSetShaderResources(uint32_t _startSlot, uint32_t _numViews, ID3D11ShaderResourceView* const* _srvs)
	{
		deferred_context_->CSSetShaderResources(_startSlot, _numViews, _srvs);
	}

	//! @brief コンピュートシェーダーのアンオーダードアクセスビューをセット
	void DeviceContext::CSSetUnorderedAccessView(uint32_t _slot, RWStructuredBuffer* _buffer)
	{
		ID3D11UnorderedAccessView* uav = _buffer->GetUAV();
		deferred_context_->CSSetUnorderedAccessViews(_slot, 1, &uav, nullptr);
	}

	//! @brief コンピュートシェーダのリソースをクリア
	void DeviceContext::CSClearResources(uint32_t _numSRVs, uint32_t _numUAVs)
	{
		ID3D11ShaderResourceView* nullSRVs[8] = {};
		deferred_context_->CSSetShaderResources(0, _numSRVs, nullSRVs);

		ID3D11UnorderedAccessView* nullUAVs[8] = {};
		deferred_context_->CSSetUnorderedAccessViews(0, _numUAVs, nullUAVs, nullptr);
	}

	//! @brief コンピュートシェーダーディスパッチ
	void DeviceContext::Dispatch(uint32_t _x, uint32_t _y, uint32_t _z)
	{
		deferred_context_->Dispatch(_x, _y, _z);
	}

	//! @brief リソースコピー
	void DeviceContext::CopyResource(StagingBuffer& _dst, RWStructuredBuffer& _src)
	{
		deferred_context_->CopyResource(_dst.GetBuffer(), _src.GetBuffer());
	}

	//! @brief 定数バッファをセット
	void DeviceContext::SetConstantBuffer(uint32_t _slot, const ConstantBuffer& _buffer)
	{
		VSSetConstantBuffer(_slot, _buffer);
		PSSetConstantBuffer(_slot, _buffer);
	}

	//! @brief ビューポートサイズをセット
	void dx3d::DeviceContext::SetViewportSize(const Rect& _size)
	{
		D3D11_VIEWPORT vp{};
		vp.Width = static_cast<float>(_size.width);
		vp.Height = static_cast<float>(_size.height);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		deferred_context_->RSSetViewports(1, &vp);
	}

	//! @brief 三角形リストを描画
	void dx3d::DeviceContext::DrawTriangleList(uint32_t _vertexCount, uint32_t _startVertexLocation)
	{
		deferred_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deferred_context_->Draw(_vertexCount, _startVertexLocation);
	}

	//! @brief インデックス付き描画
	void dx3d::DeviceContext::DrawIndexed(uint32_t _indexCount, uint32_t _startIndex, uint32_t _baseVertex)
	{
		deferred_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deferred_context_->DrawIndexed(_indexCount, _startIndex, _baseVertex);
	}


	void DeviceContext::DrawIndexedInstanced(uint32_t _indexCount, uint32_t _instanceCount, uint32_t _startIndex, uint32_t _baseVertex, uint32_t _startInstance)
	{
		deferred_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deferred_context_->DrawIndexedInstanced(_indexCount, _instanceCount, _startIndex, _baseVertex, _startInstance);
	}

	//! @brief 
	void* DeviceContext::ReadbackBuffer(StagingBuffer& _dst, RWStructuredBuffer& _src)
	{
		deferred_context_->CopyResource(_dst.GetBuffer(), _src.GetBuffer());

		D3D11_MAPPED_SUBRESOURCE mapped{};



		return mapped.pData;
	}

}