#pragma once
/**
 * @file PipelineCache.h
 * @brief パイプラインをキャッシュするクラス
 * @author Arima Keita
 * @date 2025-09-27
 */

 /*---------- インクルード ----------*/
#include <unordered_map>
#include <memory>
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Graphics/GraphicsPipelineState.h>
#include <DX3D/Graphics/VertexShaderSignature.h>
#include <DX3D/Graphics/InputLayout.h>
#include <DX3D/Graphics/PipelineKey.h>
#include <DX3D/Graphics/ShaderBinary.h>

namespace dx3d {
	/**
	 * @brief パイプラインのシェーダーファイルのパス
	 *
	 * シェーダーファイルのパスをまとめた構造体
	 */
	struct PipelineSourcePaths {
		// ---------- Vertex Shader ---------- //
		const char* vsDefault = "Assets/Shaders/Vertex/VS_Default.hlsl";
		const char* vsInstanced = "Assets/Shaders/Vertex/VS_Instanced.hlsl";
		const char* vsSpriteShadow = "Assets/Shaders/Vertex/VS_Shadow.hlsl";

		// ---------- Pixel Shader ---------- // 
		const char* psDefault = "Assets/Shaders/Pixel/PS_Default.hlsl";
	};


	/**
	 * @brief パイプラインをキャッシュするクラス
	 *
	 * パイプラインステートオブジェクトをキャッシュしておくクラス
	 */
	class PipelineCache : public GraphicsResource {
	public:
		struct PipelineCacheDesc {
			PipelineSourcePaths paths{};
		};

		explicit PipelineCache(const PipelineCacheDesc& _desc, const GraphicsResourceDesc& _gDesc)
			: GraphicsResource(_gDesc)
			, paths_(_desc.paths) {}

		GraphicsPipelineStatePtr GetOrCreate(const PipelineKey& _key);

	private:
		struct VSCacheEntry {
			ShaderBinaryPtr shader{};
			VertexShaderSignaturePtr signature{};
			InputLayoutPtr layout{};
		};

		VSCacheEntry& GetOrCreateVS(VertexShaderKind _kind);
		ShaderBinaryPtr GetOrCreatePS(PixelShaderKind _kind);


		ShaderBinaryPtr CompileFile(const char* _path, const char* _entity, ShaderBinary::Type _type);

		std::string LoadTextFile(const char* _path);
	private:

		PipelineSourcePaths paths_{};

		std::unordered_map<PipelineKey, GraphicsPipelineStatePtr, PipelineKeyHash> pso_cache_{};
		std::unordered_map<VertexShaderKind, VSCacheEntry> vs_cache_{};
		std::unordered_map<PixelShaderKind, ShaderBinaryPtr> ps_cache_{};

	};
}

