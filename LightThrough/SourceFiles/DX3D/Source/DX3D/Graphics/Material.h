#pragma once
/**
 * @file Material.h
 * @brief マテリアルの定義
 * @author Arima Keita
 * @date 2025-09-27
 */

 /*---------- インクルード ----------*/
#include <string>
#include <unordered_map>

#include <DX3D/Core/Core.h>


namespace dx3d {
	/**
	 * @brief マテリアルの定義
	 *
	 * マテリアルはシェーダーやテクスチャ、レンダーステートなどの情報を持つ
	 */
	class Material {
	public:
		void SetPipelineState(GraphicsPipelineStatePtr _pso);
		void SetConstantBuffer(const std::string& _name, ConstantBufferPtr _cb);
		void SetTexture(const std::string& _name, TexturePtr _tex);

		GraphicsPipelineStatePtr GetPipelineState() const;
		ConstantBufferPtr GetConstantBuffer(const std::string& _name) const;
		TexturePtr GetTexture(const std::string& _name) const;

	private:
		GraphicsPipelineStatePtr pso_;	// パイプラインステートオブジェクト
		std::unordered_map<std::string, ConstantBufferPtr> constant_baffers_; // 定数バッファ
		std::unordered_map<std::string, TexturePtr> textures_; // [ToDo] テクスチャ
		
	};
}