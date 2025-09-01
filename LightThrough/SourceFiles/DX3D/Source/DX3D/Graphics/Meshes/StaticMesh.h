#pragma once

/**
 * @file StaticMesh.h
 * @brief よくある図形
 * @author Arima Keita
 * @date 2025-08-22
 */

// ---------- インクルード ---------- // 
#include <DX3D/Core/Common.h>
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d {
	struct StaticMeshDesc {
		const VertexBufferDesc vb{};
		const IndexBufferDesc ib{};
	};
	/**
	 * @brief よくある図形
	 */
	class StaticMesh{
	public:
		StaticMesh(const StaticMeshDesc& _desc);
		void Draw(ID3D11DeviceContext* _ctx);

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_{};
		Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_{};
		UINT index_count_{};
	};
}