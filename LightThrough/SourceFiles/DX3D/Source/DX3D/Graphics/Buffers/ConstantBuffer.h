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

	class ConstantBuffer : public GraphicsResource {
	public:
		ConstantBuffer(const ConstantBufferDesc& _desc, const GraphicsResourceDesc& _gDesc)
			: GraphicsResource(_gDesc)
		{
			D3D11_BUFFER_DESC bd{};
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = _desc.byteWidth;
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA init{};
			D3D11_SUBRESOURCE_DATA* pInit = nullptr;
			if (_desc.initData) {
				init.pSysMem = _desc.initData;
				pInit = &init;
			}

			DX3DGraphicsLogThrowOnFail(device_.CreateBuffer(&bd, pInit, &buffer_), "ConstantBuffer CreateBufferに失敗");
		}

		//! @brief バッファ更新
		// memo: UpdateはContextの責務にしたほうがContextが動き回らなくてよかったな...反省
		void Update(DeviceContext& _cxt, const void* _data, size_t _size)
		{
			auto context = _cxt.GetDeferredContext().Get();

			D3D11_MAPPED_SUBRESOURCE mapped{};
			DX3DGraphicsLogThrowOnFail(
				context->Map(buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
				"ConstantBuffer Mapに失敗"
			);

			memcpy(mapped.pData, _data, _size);
			context->Unmap(buffer_.Get(), 0);
		}

		ID3D11Buffer* GetBuffer() const noexcept
		{
			return buffer_.Get();
		}

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_{};

	};
}