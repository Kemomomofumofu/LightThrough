/**
 * @file PipelineCache.cpp
 * @brief パイプラインステートをキャッシュするクラス
 * @author Arima Keita
 * @date 2025-09-27
 */

 // ---------- インクルード ---------- //
#include <DX3D/Graphics/PipelineCache.h>
#include <DX3D/Graphics/ShaderCache.h>
#include <DX3D/Graphics/GraphicsDevice.h>

namespace dx3d {

	GraphicsPipelineStatePtr PipelineCache::GetOrCreate(const PipelineKey& _key)
	{
		// 既に存在するならそれを返す
		if (auto it = pso_cache_.find(_key); it != pso_cache_.end()) { return it->second; }

		// ShaderCacheからシェーダーを取得
		auto& vsEntry = shader_cache_.GetVS(_key.GetVS());

		ShaderBinaryPtr psBin = nullptr;
		if ((_key.GetFlags() & PipelineFlags::ShadowPass) == 0) {
			psBin = shader_cache_.GetPS(_key.GetPS());
		}

		// パイプラインステートの定義
		GraphicsPipelineStateDesc psoDesc{
			.vs = *vsEntry.signature,
			.ps = psBin.get(),
			.inputLayout = vsEntry.layout,
			.blendMode = _key.GetBlend(),
			.depthMode = _key.GetDepth(),
		};

		// ラスタライザーステートの設定
		switch (_key.GetRaster()) {
		case RasterMode::Wireframe:
			psoDesc.rasterizerState.fillMode = FillMode::Wireframe;
			psoDesc.rasterizerState.cullMode = CullMode::None;
			break;
		case RasterMode::SolidNone:
			psoDesc.rasterizerState.cullMode = CullMode::None;
			break;
		case RasterMode::SolidBack:
			psoDesc.rasterizerState.cullMode = CullMode::Back;
			break;
		}

		// シャドウパスの場合DepthBiasを適用、通常パスではBiasを無し
		const bool isShadowPass = (_key.GetFlags() & PipelineFlags::ShadowPass) != 0 || _key.GetPS() == PixelShaderKind::None;
		if (isShadowPass) {
			psoDesc.rasterizerState.depthBias = 50;
			psoDesc.rasterizerState.slopeScaledDepthBias = 2.0f;
			psoDesc.rasterizerState.depthBiasClamp = 0.01f;
		}
		else {
			psoDesc.rasterizerState.depthBias = 0;
			psoDesc.rasterizerState.slopeScaledDepthBias = 0.0f;
			psoDesc.rasterizerState.depthBiasClamp = 0.0f;
		}


		// パイプラインステートオブジェクトの生成
		auto pso = graphics_device_->CreateGraphicsPipelineState(psoDesc);
		pso_cache_.emplace(_key, pso);

		return pso;
	}

} // namespace dx3d