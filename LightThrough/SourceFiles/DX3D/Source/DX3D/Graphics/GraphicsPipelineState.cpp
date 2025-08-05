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


dx3d::GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineStateDesc& _desc, const GraphicsResourceDesc& _gDesc)
	:GraphicsResource(_gDesc) {
	if (_desc.ps.GetType() != ShaderType::PixelShader) {
		DX3DLogThrowInvalidArg("ps が PixelShader では ありません");
	}

	auto vs = _desc.vs.GetShaderBinaryData();
	auto ps = _desc.ps.GetData();
	auto vsInputElements = _desc.vs.GetInputElementsData();

	DX3DGraphicsLogThrowOnFail(
		device_.CreateInputLayout(
			static_cast<const D3D11_INPUT_ELEMENT_DESC*>(vsInputElements.data),
			static_cast<ui32>(vsInputElements.dataSize),
			vs.data,
			vs.dataSize,
			&layout_
		),
		"CreateInputLayer を 失敗しました"
	);

	DX3DGraphicsLogThrowOnFail(
		device_.CreateVertexShader(vs.data, vs.dataSize, nullptr, &vs_),
		"CreateVertexShader を 失敗しました"
	);

	DX3DGraphicsLogThrowOnFail(
		device_.CreatePixelShader(ps.data, ps.dataSize, nullptr, &ps_),
		"CreatePixelShader を 失敗しました"
	);
}