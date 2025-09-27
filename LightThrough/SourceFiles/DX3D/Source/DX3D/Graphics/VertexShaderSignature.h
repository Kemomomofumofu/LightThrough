#pragma once
/**
 * @file VertexShaderSignature.h
 * @brief 頂点シェーダシグネチャクラス
 * @author Arima Keita
 * @date 2025-08-05
 */

// ---------- インクルード ---------- // 
#include <DX3D/Graphics/GraphicsResource.h>
#include <d3dcompiler.h>

namespace dx3d {
	/**
	 * @brief 頂点シェーダシグネチャクラス
	 *
	 * 頂点データのInputLayoutを生成するために必要な情報を持つクラス
	 */
	
	class VertexShaderSignature final : public GraphicsResource {
	public :
		VertexShaderSignature(const VertexShaderSignatureDesc& _desc, const GraphicsResourceDesc& _gDesc);
		BinaryData GetShaderBinaryData() const noexcept;
		ID3D11ShaderReflection* GetReflection() const noexcept;

	private:
		ShaderBinaryPtr vs_binary_{};	// バイナリデータ
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> shader_reflection_{};	// リフレクション
	};
}