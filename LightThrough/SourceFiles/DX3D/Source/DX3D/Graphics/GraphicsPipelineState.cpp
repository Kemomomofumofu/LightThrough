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


dx3d::GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineStateDesc& _desc, const GraphicsResourceDesc& _gDesc)
	:GraphicsResource(_gDesc) {
	if (_desc.ps.GetType() != ShaderBinary::Type::Pixel) {
		DX3DLogThrowInvalidArg("ps が PixelShader では ありません");
	}

	// VertexShader
	auto vs = _desc.vs.GetShaderBinaryData();
	DX3DGraphicsLogThrowOnFail(
		device_.CreateVertexShader(vs.data, vs.dataSize, nullptr, &vs_),
		"CreateVertexShader を 失敗しました"
	);

	// PixelShader
	auto ps = _desc.ps.GetData();
	DX3DGraphicsLogThrowOnFail(
		device_.CreatePixelShader(ps.data, ps.dataSize, nullptr, &ps_),
		"CreatePixelShader を 失敗しました"
	);

	// InputLayout
	layout_ = _desc.inputLayout->Get();	// 参照を保持
}