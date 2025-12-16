#pragma once

/**
 * @file ShaderBinary.h
 * @brief シェーダのタイプやバイナリデータを保持する
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d {
	//! シェーダタイプ
	class ShaderBinary final : public GraphicsResource {
	public:
	enum Type {
		Unknown,     //!< 不明
		Vertex,      //!< 頂点シェーダー
		Pixel,       //!< ピクセルシェーダー
		Compute,     //!< コンピュートシェーダー
		//Geometry,    //!< ジオメトリシェーダー
		//Hull,        //!< ハルシェーダー
		//Domain,      //!< ドメインシェーダー
	};

	//! シェーダコンパイル設定構造体
	struct ShaderCompileDesc {
		const char* shaderSourceName{};
		const  void* shaderSourceCode{};
		size_t shaderSourceCodeSize{};
		const char* shaderEntryPoint{};
		Type shaderType{};
	};


		ShaderBinary(const ShaderCompileDesc& _desc, const GraphicsResourceDesc& _gDesc);
		BinaryData GetData() const noexcept;
		Type GetType() const noexcept;

	private:
		Microsoft::WRL::ComPtr<ID3DBlob> blob_{};
		Type type_{};
	};
}