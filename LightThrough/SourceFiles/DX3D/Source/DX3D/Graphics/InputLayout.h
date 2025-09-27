#pragma once
/**
 * @file InputLayout.h
 * @brief InputLayout ラッパ & 自動生成
 */

#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Graphics/VertexShaderSignature.h>
#include <wrl/client.h>

namespace dx3d {

	struct InputLayoutDesc {
		VertexShaderSignaturePtr signature;   ///< 解析対象の頂点シェーダシグネチャ
		const char* instancePrefix = "INSTANCE_";	///< インスタンスデータのセマンティクスのプレフィックス
	};

	class InputLayout final : public GraphicsResource {
	public:
		InputLayout(const InputLayoutDesc& _desc, const GraphicsResourceDesc& _gDesc);

		ID3D11InputLayout* Get() const noexcept { return layout_.Get(); }
		uint32_t GetVertexStride() const noexcept { return vertex_stride_; }
		uint32_t GetInstanceStride() const noexcept { return instance_stride_; }
		uint32_t GetElementCount() const noexcept { return num_elements_; }
		const D3D11_INPUT_ELEMENT_DESC* GetElements() const noexcept { return elements_.data(); }

	private:
		void Build(const InputLayoutDesc& _desc);

	private:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_{};
		std::vector<D3D11_INPUT_ELEMENT_DESC> elements_{};
		uint32_t vertex_stride_{};
		uint32_t instance_stride_{};
		uint32_t num_elements_{};
	};

}
