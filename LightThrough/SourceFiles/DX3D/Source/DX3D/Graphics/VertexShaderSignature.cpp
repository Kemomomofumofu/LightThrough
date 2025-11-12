/**
 * @file VertexShaderSignature.h
 * @brief 
 * @author Arima Keita
 * @date 2025-08-05
 */

// ---------- インクルード ---------- // 
#include <d3dcompiler.h>
#include <ranges>
#include <DX3D/Graphics/VertexShaderSignature.h>
#include <DX3D/Graphics/ShaderBinary.h>
#include <DX3D/Graphics/GraphicsUtils.h>

/**
 * @brief コンストラクタ
 * @param _desc 頂点シェーダシグネチャの定義
 * @param _gDesc リソースの定義
 */
dx3d::VertexShaderSignature::VertexShaderSignature(const VertexShaderSignatureDesc& _desc, const GraphicsResourceDesc& _gDesc)
	: GraphicsResource(_gDesc)
	, vs_binary_(_desc.vsBinary)
{
	if (!vs_binary_) {
		DX3DLogThrowInvalidArg("[VertexShaderSignature] シェーダバイナリが存在しません");
	}
	if (vs_binary_->GetType() != ShaderBinary::Type::Vertex) {
		DX3DLogThrowInvalidArg("[VertexShaderSignature] vsBinaryのタイプがVertexShaderではありません");
	}

	auto vsData = vs_binary_->GetData();

	// リフレクションの生成
	DX3DGraphicsLogThrowOnFail(
		D3DReflect(
			vsData.data,
			vsData.dataSize,
			IID_PPV_ARGS(&shader_reflection_)
		),
		"[VertexShaderSignature] D3DReflectを失敗"
	);
}

/**
 * @brief シェーダバイナリを取得する
 * @return シェーダバイナリ
 */
dx3d::BinaryData dx3d::VertexShaderSignature::GetShaderBinaryData() const noexcept
{
	return vs_binary_->GetData();
}

/**
 * @brief リフレクションを取得する
 * @return リフレクションのポインタ
 */
ID3D11ShaderReflection* dx3d::VertexShaderSignature::GetReflection() const noexcept
{
	return shader_reflection_.Get();
}

