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
		ui32 GetVertexListSize() const noexcept;
		
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_{};
		ui32 vertex_size_{};
		ui32 vertex_list_size_{};

		friend class DeviceContext;
	};
}