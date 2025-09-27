/**
 * @file InputLayout.cpp
 * @brief InputLayout 自動生成実装
 */

#include <DX3D/Graphics/InputLayout.h>
#include <DX3D/Graphics/GraphicsLogUtils.h>
#include <DX3D/Graphics/GraphicsUtils.h>
#include <cassert>
#include <cstring>

namespace dx3d {

	/**
	 * @brief DXGI_FORMATのバイト数を取得
	 * @param f DXGI_FORMAT
	 * @return バイト数
	 */
	static uint32_t GetFormatSize(DXGI_FORMAT f)
	{
		switch (f) {
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R32_UINT: return 4;
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G32_UINT: return 8;
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_SINT:
		case DXGI_FORMAT_R32G32B32_UINT: return 12;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
		case DXGI_FORMAT_R32G32B32A32_UINT: return 16;
		default:
			assert(false && "Unhandled DXGI_FORMAT size");
			return 0;
		}
	}

	/**
	 * @brief コンストラクタ
	 * @param _desc InputLayoutの定義
	 * @param _gDesc グラフィックリソースの定義
	 */
	InputLayout::InputLayout(const InputLayoutDesc& _desc, const GraphicsResourceDesc& _gDesc)
		: GraphicsResource(_gDesc)
	{
		Build(_desc);
	}


	/**
	 * @brief InputLayoutのビルド
	 * @param _desc InputLayoutの定義
	 */
	void InputLayout::Build(const InputLayoutDesc& _desc)
	{
		if (!_desc.signature) {
			DX3DLogThrowInvalidArg("InputLayoutDesc.signature が nullptr です");
		}

		// 頂点シェーダのバイナリデータを取得
		auto vsData = _desc.signature->GetShaderBinaryData();

		// vsSignatureのreflectionを利用
		ID3D11ShaderReflection* refl = _desc.signature->GetReflection();
		if (!refl) {
			DX3DLogThrowInvalidArg("VertexShaderSignature のリフレクションが無効です");
		}

		const char* instPrefix = _desc.instancePrefix;
		size_t instPrefixLen = (instPrefix && instPrefix[0] != '\0') ? std::strlen(instPrefix) : 0;

		D3D11_SHADER_DESC shaderDesc{};
		DX3DGraphicsLogThrowOnFail(refl->GetDesc(&shaderDesc), "ShaderReflection::GetDesc 失敗");

		elements_.reserve(shaderDesc.InputParameters);

		uint32_t slotOffset[2] = { 0u, 0u };

		for (UINT i = 0; i < shaderDesc.InputParameters; ++i) {
			D3D11_SIGNATURE_PARAMETER_DESC p{};
			DX3DGraphicsLogThrowOnFail(refl->GetInputParameterDesc(i, &p), "GetInputParameterDesc 失敗");

			// System semantic はスキップ (SV_*)
			if (p.SystemValueType != D3D_NAME_UNDEFINED) {
				continue;
			}

			D3D11_INPUT_ELEMENT_DESC e{};
			e.SemanticName = p.SemanticName;
			e.SemanticIndex = p.SemanticIndex;
			e.Format = GraphicsUtils::GetDXGIFormatFromMask(p.ComponentType, p.Mask);

			bool isInstance = false;
			if (instPrefixLen > 0) {
				// インスタンスデータかどうかをセマンティクス名で判定
				isInstance = (std::strncmp(p.SemanticName, instPrefix,instPrefixLen) == 0);
			}
			e.InputSlot = isInstance ? 1 : 0;
			e.AlignedByteOffset = slotOffset[e.InputSlot];
			e.InputSlotClass = isInstance ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
			e.InstanceDataStepRate = isInstance ? 1 : 0;

			slotOffset[e.InputSlot] += GetFormatSize(e.Format);
			elements_.push_back(e);
		}

		vertex_stride_ = slotOffset[0];
		instance_stride_ = slotOffset[1];
		num_elements_ = static_cast<uint32_t>(elements_.size());

		// InputLayoutの生成
		DX3DGraphicsLogThrowOnFail(
			device_.CreateInputLayout(
				elements_.data(),
				num_elements_,
				vsData.data,
				vsData.dataSize,
				&layout_),
			"CreateInputLayout 失敗"
		);
	}

}
