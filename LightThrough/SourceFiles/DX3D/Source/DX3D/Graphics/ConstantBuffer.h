#pragma once
/**
 * @file ConstantBuffer.h
 * @brief 定数バッファ
 * @author Arima Keita
 * @date 2025-09-05
 */

 // ---------- インクルード ---------- // 
#include <DirectXMath.h>
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Graphics/DeviceContext.h>

namespace dx3d {
	struct CBPerFrame {
		DirectX::XMMATRIX view;	// ビュー行列
		DirectX::XMMATRIX proj;	// プロジェクション行列
	};

	struct CBPerObject {
		DirectX::XMMATRIX world;	// ワールド行列
	};

	template <typename T >
	class ConstantBuffer : public GraphicsResource {
	public:
		ConstantBuffer(const GraphicsResourceDesc& _desc)
			: GraphicsResource(_desc)
		{
			D3D11_BUFFER_DESC bd{};
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(T);
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			DX3DGraphicsLogThrowOnFail(device_.CreateBuffer(&bd, nullptr, &buffer_), "ConstantBuffer CreateBufferに失敗");
		}


		void Update(DeviceContext& _cxt, const T& _data)
		{
			auto context = _cxt.GetDeviceContext().Get();

			D3D11_MAPPED_SUBRESOURCE mapped{};
			DX3DGraphicsLogThrowOnFail(
				context.Map(buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
				"ConstantBuffer Mapに失敗"
			);

			memcpy(mapped.pData, &_data, sizeof(T));
			context.Unmap(buffer_.Get(), 0);
		}

		ID3D11Buffer* Get() const noexcept { return buffer_.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_{};
	};
}