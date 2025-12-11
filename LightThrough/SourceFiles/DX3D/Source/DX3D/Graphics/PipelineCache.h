#pragma once
/**
 * @file PipelineCache.h
 * @brief パイプラインステートをキャッシュするクラス
 * @author Arima Keita
 * @date 2025-09-27
 */

 /*---------- インクルード ----------*/
#include <unordered_map>
#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Graphics/GraphicsPipelineState.h>
#include <DX3D/Graphics/PipelineKey.h>

namespace dx3d {
	class ShaderCache;

	/**
	 * @brief パイプラインステートをキャッシュするクラス
	 */
	class PipelineCache : public GraphicsResource {
	public:
		struct PipelineCacheDesc {
			ShaderCache& shaderCache;
		};

		explicit PipelineCache(const PipelineCacheDesc& _desc, const GraphicsResourceDesc& _gDesc)
			: GraphicsResource(_gDesc)
			, shader_cache_(_desc.shaderCache)
		{
		}

		GraphicsPipelineStatePtr GetOrCreate(const PipelineKey& _key);

	private:
		ShaderCache& shader_cache_;
		std::unordered_map<PipelineKey, GraphicsPipelineStatePtr, PipelineKeyHash> pso_cache_{};
	};

} // namespace dx3d