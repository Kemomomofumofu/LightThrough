#pragma once
/**
 * @file GraphicsPipelineState.h
 * @brief パイプラインで使用されるシェーダーを保持するクラス
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Graphics/PipelineKey.h>

namespace dx3d {
	enum class FillMode {
		Solid,
		Wireframe,
	};
	enum class CullMode {
		None,
		Front,
		Back,
	};
	struct RasterizerStateDesc {
		FillMode fillMode = FillMode::Solid;
		CullMode cullMode = CullMode::Back;
		bool frontCounterClockwise = true;
		int depthBias = 1;
		float slopeScaledDepthBias = 3.0f;
		float depthBiasClamp = 0.0f;
		bool depthClipEnable = true;
		bool scissorEnable = false;
		bool multiSampleEnable = false;
		bool antiAliasedLineEnable = false;
	};
	
	struct GraphicsPipelineStateDesc {
		const VertexShaderSignature& vs;
		const ShaderBinary* ps{};
		InputLayoutPtr inputLayout{};
		RasterizerStateDesc rasterizerState;
		BlendMode blendMode = BlendMode::Opaque;
		DepthMode depthMode = DepthMode::Default;
	};


	/**
	 * @brief グラフィックパイプラインステートクラス
	 *
	 * VertexShaderやPixelShader、それに対するInputLayoutといったパイプラインに関するものを保持するクラス
	 */
	class GraphicsPipelineState final : public GraphicsResource {
	public:
		GraphicsPipelineState(const GraphicsPipelineStateDesc& _desc, const GraphicsResourceDesc& _gDesc);

		/**
		 * @brief パイプラインステートを適用する
		 * @param _context 
		 */
		void Apply(ID3D11DeviceContext* _context) const;
	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vs_{};
		Microsoft::WRL::ComPtr<ID3D11PixelShader>  ps_{};
		Microsoft::WRL::ComPtr<ID3D11InputLayout>  layout_{};
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rast_state_{};
		Microsoft::WRL::ComPtr<ID3D11BlendState> blend_state_{};
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_state_{};

		friend class DeviceContext;
	};
}