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
	if (!_desc.shaderSourceName) {
		DX3DLogThrowInvalidArg("ShaderSourceName が 存在しません");
	}
	if (!_desc.shaderSourceCode) {
		DX3DLogThrowInvalidArg("ShaderSourceCode が 存在しません");
	}
	if (!_desc.shaderSourceCodeSize) {
		DX3DLogThrowInvalidArg("ShaderSourceCodeSize が 存在しません");
	}
	if (!_desc.shaderEntryPoint) {
		DX3DLogThrowInvalidArg("ShaderEntryPoint が 存在しません");
	}

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
			nullptr,
			_desc.shaderEntryPoint,
			dx3d::GraphicsUtils::GetShaderModelTarget(_desc.shaderType),
			compileFlags,
			0,
			&blob_,
			&errorBlob
		),
		errorBlob.Get()
	);

}

dx3d::ShaderBinaryData dx3d::ShaderBinary::GetData() const noexcept
{
	return {
		blob_->GetBufferPointer(),
		blob_->GetBufferSize()
	};
}

dx3d::ShaderType dx3d::ShaderBinary::GetType() const noexcept
{
	return type_;
}
