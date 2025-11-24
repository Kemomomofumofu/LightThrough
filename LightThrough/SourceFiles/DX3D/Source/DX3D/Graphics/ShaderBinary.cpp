/**
 * @file ShaderBinary.cpp
 * @brief シェーダのバイナリオブジェクトとタイプを保持する
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/ShaderBinary.h>
#include <DX3D/Graphics/GraphicsUtils.h>
#include <d3dcompiler.h>

/**
 * @brief コンストラクタ
 * @param _desc シェーダバイナリの設定
 * @param _gDesc グラフィックリソースの設定
 */
dx3d::ShaderBinary::ShaderBinary(const ShaderCompileDesc& _desc, const GraphicsResourceDesc& _gDesc)
	: GraphicsResource(_gDesc),
	type_(_desc.shaderType)
{
	UINT compileFlags{};

#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG;
#endif

	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob{};
	DX3DGraphicsCheckShaderCompile(
		D3DCompile(
			_desc.shaderSourceCode,
			_desc.shaderSourceCodeSize,
			_desc.shaderSourceName,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			_desc.shaderEntryPoint,
			dx3d::GraphicsUtils::GetShaderModelTarget(_desc.shaderType),
			compileFlags,
			0,
			blob_.GetAddressOf(),
			errorBlob.GetAddressOf()
		),
		errorBlob.Get()
	);

}

dx3d::BinaryData dx3d::ShaderBinary::GetData() const noexcept
{
	return {
		blob_->GetBufferPointer(),
		blob_->GetBufferSize()
	};
}

dx3d::ShaderBinary::Type dx3d::ShaderBinary::GetType() const noexcept
{
	return type_;
}
