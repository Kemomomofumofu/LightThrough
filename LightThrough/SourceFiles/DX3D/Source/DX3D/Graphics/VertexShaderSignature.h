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
	 * 頂点シェーダのバイナリデータからレイアウトの情報を解析して作成したりする。
	 */

	class VertexShaderSignature final : public GraphicsResource {
	public :
		VertexShaderSignature(const VertexShaderSignatureDesc& _desc, const GraphicsResourceDesc& _gDesc);
		BinaryData GetShaderBinaryData() const noexcept;
		BinaryData GetInputElementsData() const noexcept;

	private:
		ShaderBinaryPtr vs_binary_{};	// バイナリデータ
		Microsoft::WRL::ComPtr<ID3D11ShaderReflection> shader_reflection_{};	// リフレクション
		D3D11_INPUT_ELEMENT_DESC elements_[D3D11_STANDARD_VERTEX_ELEMENT_COUNT]{};	// インプットの情報
		ui32 num_elements_{};	// エレメントの量
	};
}