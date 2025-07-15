/**
 * @file GraphicsPipelineState.cpp
 * @brief グラフィックパイプラインステート
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/GraphicsPipelineState.h>
#include <DX3D/Graphics/ShaderBinary.h>

dx3d::GraphicsPipelineState::GraphicsPipelineState(const GraphicsPipelineStateDesc& _desc, const GraphicsResourceDesc& _gDesc)
	:GraphicsResource(_gDesc) {
	if (_desc.vs.GetType() != ShaderType::VertexShader) {
		DX3DLogThrowInvalidArg("vs が VertexShader では ありません");
	}
	if (_desc.ps.GetType() != ShaderType::PixelShader) {
		DX3DLogThrowInvalidArg("ps が PixelShader では ありません");
	}

	auto vs = _desc.vs.GetData();
	auto ps = _desc.ps.GetData();

	constexpr D3D11_INPUT_ELEMENT_DESC elements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	DX3DGraphicsLogThrowOnFail(device_.CreateInputLayout(elements, std::size(elements), vs.data, vs.dataSize, &layout_), "CreateInputLayer を 失敗しました");
	DX3DGraphicsLogThrowOnFail(device_.CreateVertexShader(vs.data, vs.dataSize, nullptr, &vs_), "CreateVertexShader を 失敗しました");
	DX3DGraphicsLogThrowOnFail(device_.CreatePixelShader(ps.data, ps.dataSize, nullptr, &ps_), "CreatePixelShader を 失敗しました");
}