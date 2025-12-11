/**
 * @file GraphicsPipelineState.cpp
 * @brief グラフィックパイプラインステート
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/GraphicsPipelineState.h>
#include <DX3D/Graphics/ShaderBinary.h>
#include <DX3D/Graphics/VertexShaderSignature.h>
#include <DX3D/Graphics/InputLayout.h>


namespace dx3d {
	dx3d::GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineStateDesc& _desc, const GraphicsResourceDesc& _gDesc)
		:GraphicsResource(_gDesc)
	{
		// VertexShader
		auto vs = _desc.vs.GetShaderBinaryData();
		DX3DGraphicsLogThrowOnFail(
			device_.CreateVertexShader(vs.data, vs.dataSize, nullptr, &vs_),
			"CreateVertexShader を 失敗しました"
		);

		// PixelShader
		if (_desc.ps) {
			auto ps = _desc.ps->GetData();
			DX3DGraphicsLogThrowOnFail(
				device_.CreatePixelShader(ps.data, ps.dataSize, nullptr, &ps_),
				"CreatePixelShader を 失敗しました"
			);
		}

		// InputLayout
		layout_ = _desc.inputLayout->Get();	// 参照を保持

		// RasterizerState
		D3D11_RASTERIZER_DESC rastDesc{};
		rastDesc.FillMode = (_desc.rasterizerState.fillMode == FillMode::Solid) ? D3D11_FILL_MODE::D3D11_FILL_SOLID : D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;

		switch (_desc.rasterizerState.cullMode) {
		case CullMode::None:
			rastDesc.CullMode = D3D11_CULL_NONE;
			break;
		case CullMode::Front:
			rastDesc.CullMode = D3D11_CULL_FRONT;
			break;
		case CullMode::Back:
			rastDesc.CullMode = D3D11_CULL_BACK;
			break;
		}

		rastDesc.FrontCounterClockwise = _desc.rasterizerState.frontCounterClockwise;
		rastDesc.DepthBias = _desc.rasterizerState.depthBias;
		rastDesc.DepthBiasClamp = _desc.rasterizerState.depthBiasClamp;
		rastDesc.SlopeScaledDepthBias = _desc.rasterizerState.slopeScaledDepthBias;
		rastDesc.DepthClipEnable = _desc.rasterizerState.depthClipEnable;
		rastDesc.ScissorEnable = _desc.rasterizerState.scissorEnable;
		rastDesc.MultisampleEnable = _desc.rasterizerState.multiSampleEnable;
		rastDesc.AntialiasedLineEnable = _desc.rasterizerState.antiAliasedLineEnable;
		DX3DGraphicsLogThrowOnFail(
			device_.CreateRasterizerState(&rastDesc, &rast_state_),
			"CreateRasterizerState が失敗しました"
		);


		// BlendState
		D3D11_BLEND_DESC blendDesc{};
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		switch (_desc.blendMode) {
		case BlendMode::Alpha:
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			break;
		case BlendMode::Add:
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // 'D3D_BLEND_SRC_ALPHA' or 'D3D11_BLEND_ONE'
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			break;
		case BlendMode::Opaque:
		default:
			blendDesc.RenderTarget[0].BlendEnable = false;
			break;
		}
		
		DX3DGraphicsLogThrowOnFail(
			device_.CreateBlendState(&blendDesc, &blend_state_),
			"CreateBlendState が失敗しました"
		);
	}
}
