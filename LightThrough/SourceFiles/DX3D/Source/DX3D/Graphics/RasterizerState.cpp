/**
 * @file RasterizerState.cpp
 * @brief ラスタライザーのステートを保持する
 * @author Arima Keita
 * @date 2025-09-04
 */

// ---------- インクルード ---------- // 
#include <DX3D/Graphics/RasterizerState.h>

dx3d::RasterizerState::RasterizerState(const RasterizerStateDesc& _desc, const GraphicsResourceDesc& _gDesc)
	: GraphicsResource(_gDesc)
{
	D3D11_RASTERIZER_DESC rastDesc{};
	rastDesc.FillMode = (_desc.fillMode == FillMode::Solid) ? D3D11_FILL_MODE::D3D11_FILL_SOLID : D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;

	switch (_desc.cullMode) {
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

	rastDesc.FrontCounterClockwise = _desc.frontCounterClockwise;
	rastDesc.DepthClipEnable = _desc.depthClipEnable;
	rastDesc.ScissorEnable = _desc.scissorEnable;
	rastDesc.MultisampleEnable = _desc.multiSampleEnable;
	rastDesc.AntialiasedLineEnable = _desc.antiAliasedLineEnable;

	DX3DGraphicsLogThrowOnFail(
		device_.CreateRasterizerState(&rastDesc, &rast_state_),
		"CreateRasterizerState が失敗しました"
	);
}
