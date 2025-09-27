
/**
 * @file IndexBuffer.cpp
 * @brief インデックスバッファクラス
 * @author Arima Keita
 * @date 2025-09-03
 */

// ---------- インクルード ---------- // 
#include <DX3D/Graphics/Buffers/IndexBuffer.h>

dx3d::IndexBuffer::IndexBuffer(const IndexBufferDesc& _desc, const GraphicsResourceDesc& _gDesc)
	: index_count_(_desc.indexCount)
	, GraphicsResource(_gDesc)
{
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(uint32_t) * _desc.indexCount;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = _desc.indexList;

	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer{};

	DX3DGraphicsLogThrowOnFail(device_.CreateBuffer(&bd, &initData, &buffer), "IndexBuffer CreateBufferに失敗しました");

	buffer_ = buffer;
}

ID3D11Buffer* dx3d::IndexBuffer::GetBuffer() const noexcept
{
	return buffer_.Get();
}

uint32_t dx3d::IndexBuffer::GetIndexCount() const noexcept
{
	return index_count_;
}
