/**
 * @file VertexBuffer.cpp
 * @brief 頂点バッファの定義
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/Buffers/VertexBuffer.h>


namespace dx3d
{

	/**
	 * @brief コンストラクタ
	 * @param _desc 頂点バッファの設定
	 * @param _gDesc グラフィックリソースの設定
	 */
	VertexBuffer::VertexBuffer(const VertexBufferDesc& _desc, const GraphicsResourceDesc& _gDesc)
		:GraphicsResource(_gDesc),
		vertex_size_(_desc.vertexSize),
		vertex_list_size_(_desc.vertexListSize)
	{
		if (!_desc.vertexList) {
			DX3DLogThrowInvalidArg("VertexList が 存在しません");
		}
		if (!_desc.vertexListSize) {
			DX3DLogThrowInvalidArg("VertexListSize が 0 です");
		}
		if (!_desc.vertexSize) {
			DX3DLogThrowInvalidArg("VertexSize が 0 です");
		}

		D3D11_BUFFER_DESC buffDesc{};
		// 全体のサイズを計算
		buffDesc.ByteWidth = _desc.vertexListSize * _desc.vertexSize;
		buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = _desc.vertexList;

		DX3DGraphicsLogThrowOnFail(device_.CreateBuffer(&buffDesc, &initData, &buffer_), "VertexBuffer CreateBufferに失敗しました");
	}

	ID3D11Buffer* VertexBuffer::GetBuffer() const noexcept
	{
		return buffer_.Get();
	}

	ui32 VertexBuffer::GetVertexSize() const noexcept
	{
		return vertex_size_;
	}

	ui32 VertexBuffer::GetVertexListSize() const noexcept
	{
		return vertex_list_size_;
	}
}
