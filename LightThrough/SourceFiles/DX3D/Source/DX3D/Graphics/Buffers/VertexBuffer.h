#pragma once
/**
 * @file VertexBuffer.h
 * @brief 頂点バッファクラスの定義をする
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/GraphicsResource.h>


 /**
  * @brief 頂点バッファクラス
  *
  * 頂点情報のリストやサイズを保持しているクラス
  */
namespace dx3d {
	class VertexBuffer final : public GraphicsResource {
	public:
		VertexBuffer(const VertexBufferDesc& _desc, const GraphicsResourceDesc& _gDesc);
		ID3D11Buffer* GetBuffer() const noexcept;
		uint32_t GetVertexSize() const noexcept;
		uint32_t GetVertexListSize() const noexcept;

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_{};
		uint32_t vertex_size_{};
		uint32_t vertex_list_size_{};

		friend class DeviceContext;
	};
}