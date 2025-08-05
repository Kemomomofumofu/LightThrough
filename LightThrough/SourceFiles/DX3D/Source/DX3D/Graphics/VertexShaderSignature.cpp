/**
 * @file VertexShaderSignature.h
 * @brief 
 * @author Arima Keita
 * @date 2025-08-05
 */

// ---------- インクルード ---------- // 
#include <DX3D/Graphics/VertexShaderSignature.h>
#include <DX3D/Graphics/ShaderBinary.h>
#include <d3dcompiler.h>
#include <ranges>
#include <DX3D/Graphics/GraphicsUtils.h>

/**
 * @brief コンストラクタ
 * @param _gDesc リソースの定義
 */
dx3d::VertexShaderSignature::VertexShaderSignature(const VertexShaderSignatureDesc& _desc, const GraphicsResourceDesc& _gDesc)
	: GraphicsResource(_gDesc)
	, vs_binary_(_desc.vsBinary)
{
	if (!_desc.vsBinary) {
		DX3DLogThrowInvalidArg("シェーダバイナリが存在しません");
	}
	if (_desc.vsBinary->GetType() != ShaderType::VertexShader) {
		DX3DLogThrowInvalidArg("vsBinaryのタイプがVertexShaderではありません");
	}

	auto vsData = vs_binary_->GetData();

	DX3DGraphicsLogThrowOnFail(
		D3DReflect(
			vsData.data,
			vsData.dataSize,
			IID_PPV_ARGS(&shader_reflection_)
		),
		"D3DReflectを失敗"
	);

	D3D11_SHADER_DESC shaderDesc{};
	DX3DGraphicsLogThrowOnFail(
		shader_reflection_->GetDesc(&shaderDesc),
		"ID3D11ShaderReflection::GetDescを失敗"
	);

	num_elements_ = shaderDesc.InputParameters;

	D3D11_SIGNATURE_PARAMETER_DESC params[D3D11_STANDARD_VERTEX_ELEMENT_COUNT]{};
	for (auto i : std::views::iota(0u, num_elements_)) {
		DX3DGraphicsLogThrowOnFail(
			shader_reflection_->GetInputParameterDesc(i, &params[i]),
			"ID3D11ShaderReflection::GetInputParameterDescを失敗"
		);
	}
	
	// これ
	//constexpr D3D11_INPUT_ELEMENT_DESC elements[] = {
	//{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	//{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	//};
	for (auto i : std::views::iota(0u, num_elements_)) {
		auto param = params[i];
		elements_[i] = {
			param.SemanticName,
			param.SemanticIndex,
			GraphicsUtils::GetDXGIFormatFromMask(param.ComponentType, param.Mask),
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		};
	}

}

dx3d::BinaryData dx3d::VertexShaderSignature::GetShaderBinaryData() const noexcept
{
	return vs_binary_->GetData();
}

dx3d::BinaryData dx3d::VertexShaderSignature::GetInputElementsData() const noexcept
{
	return {
		elements_,
		num_elements_
	};
}
