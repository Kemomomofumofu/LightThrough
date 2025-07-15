#pragma once
/**
 * @file GraphicsPipelineState.h
 * @brief パイプラインで使用されるシェーダーを保持するクラス
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d {

	/**
	 * @brief グラフィックパイプラインステートクラス
	 *
	 * VertexShaderやPixelShader、それに対するInputLayoutといったパイプラインに関するものを保持するクラス
	 */
	class GraphicsPipelineState final : public GraphicsResource {
	public:
		GraphicsPipelineState(const GraphicsPipelineStateDesc& _desc, const GraphicsResourceDesc& _gDesc);
	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vs_{};
		Microsoft::WRL::ComPtr<ID3D11PixelShader>  ps_{};
		Microsoft::WRL::ComPtr<ID3D11InputLayout>  layout_{};

		friend class DeviceContext;
	};
}