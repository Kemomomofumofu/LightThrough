#pragma once
/**
 * @file ShaderCache.h
 * @brief シェーダーをキャッシュするクラス
 */

 /*---------- インクルード ----------*/
#include <unordered_map>
#include <wrl/client.h>
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Graphics/ShaderBinary.h>
#include <DX3D/Graphics/VertexShaderSignature.h>
#include <DX3D/Graphics/InputLayout.h>

namespace dx3d {

	// シェーダーの種類
	enum class VertexShaderKind : uint8_t;
	enum class PixelShaderKind : uint8_t;
	enum class ComputeShaderKind : uint8_t;

	/**
	 * @brief シェーダーファイルのパス
	 */
	struct ShaderSourcePaths {
		// Vertex Shader
		const char* vsDefault = "Assets/Shaders/Vertex/VS_Default.hlsl";
		const char* vsInstanced = "Assets/Shaders/Vertex/VS_Instanced.hlsl";
		const char* vsShadow = "Assets/Shaders/Vertex/VS_Shadow.hlsl";

		// Pixel Shader
		const char* psDefault = "Assets/Shaders/Pixel/PS_Default.hlsl";

		// Compute Shader
		const char* csShadowTest = "Assets/Shaders/Compute/CS_ShadowTest.hlsl";
	};

	/**
	 * @brief シェーダーをキャッシュするクラス
	 */
	class ShaderCache : public GraphicsResource {
	public:
		// VS用エントリ（Signature + Layout も一緒に持つ）
		struct VSEntry {
			ShaderBinaryPtr binary{};
			VertexShaderSignaturePtr signature{};
			InputLayoutPtr layout{};
		};

		// CS用エントリ
		struct CSEntry {
			ShaderBinaryPtr binary{};
			Microsoft::WRL::ComPtr<ID3D11ComputeShader> shader{};
		};

		struct ShaderCacheDesc {
			ShaderSourcePaths paths{};
		};

		explicit ShaderCache(const ShaderCacheDesc& _desc, const GraphicsResourceDesc& _gDesc)
			: GraphicsResource(_gDesc)
			, paths_(_desc.paths) {
		}

		// 各シェーダーの取得
		VSEntry& GetVS(VertexShaderKind _kind);
		ShaderBinaryPtr GetPS(PixelShaderKind _kind);
		CSEntry& GetCS(ComputeShaderKind _kind);

	private:
		ShaderBinaryPtr CompileFile(const char* _path, const char* _entry, ShaderBinary::Type _type);
		std::string LoadTextFile(const char* _path);

	private:
		ShaderSourcePaths paths_{};

		std::unordered_map<VertexShaderKind, VSEntry> vs_cache_{};
		std::unordered_map<PixelShaderKind, ShaderBinaryPtr> ps_cache_{};
		std::unordered_map<ComputeShaderKind, CSEntry> cs_cache_{};
	};

} // namespace dx3d