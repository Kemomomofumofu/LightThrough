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
	class ShaderBinary final : public GraphicsResource {
	public:
		ShaderBinary(const ShaderCompileDesc& _desc, const GraphicsResourceDesc& _gDesc);
		ShaderBinaryData GetData() const noexcept;
		ShaderType GetType() const noexcept;

	private:
		Microsoft::WRL::ComPtr<ID3DBlob> blob_{};
		ShaderType type_{};
	};
}