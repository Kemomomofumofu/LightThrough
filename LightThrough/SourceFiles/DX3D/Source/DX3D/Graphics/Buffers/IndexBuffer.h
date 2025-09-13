#pragma once
/**
 * @file IndexBuffer.h
 * @brief インデックスバッファクラス
 * @author Arima Keita
 * @date 2025-09-03
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d {
	class IndexBuffer : public GraphicsResource {
	public:
		IndexBuffer( const IndexBufferDesc& _desc, const GraphicsResourceDesc& _gDesc);

		ID3D11Buffer* GetBuffer() const noexcept;
		ui32 GetIndexCount() const noexcept;

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_{};
		ui32 index_count_{};
	};
}