/**
 * @file Material.cpp
 * @brief _aadd_i32
 * @author Arima Keita
 * @date 2025-09-27
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/Material.h>

namespace dx3d {

	/**
	 * @brief PipelineStateのSetter
	 * @param _pso PipelineStateへのポインタ
	 */
	void Material::SetPipelineState(GraphicsPipelineStatePtr _pso)
	{
		pso_ = _pso;
	}

	/**
	 * @brief ConstantBufferを設定
	 * @param _name	ConstantBufferの名前
	 * @param _cb	ConstantBufferへのポインタ
	 */
	void Material::SetConstantBuffer(const std::string& _name, ConstantBufferPtr _cb)
	{
		constant_baffers_[_name] = _cb;
	}

	void Material::SetTexture(const std::string& _name, TexturePtr _tex)
	{
		textures_[_name] = _tex;
	}

	/**
	 * @brief PipelineStateのGetter
	 * @return PipelineStateへのポインタ
	 */
	GraphicsPipelineStatePtr Material::GetPipelineState() const
	{
		return pso_;
	}

	/**
	 * @brief ConstantBufferのGetter
	 * @param _name	ConstantBufferの名前
	 * @return Bufferへのポインタ, 存在しない場合: nullptr
	 */
	ConstantBufferPtr Material::GetConstantBuffer(const std::string& _name) const
	{
		if (auto it = constant_baffers_.find(_name); it != constant_baffers_.end()) {
			return it->second;
		}
		return nullptr;
	}

	/**
	 * @brief TextureのGetter
	 * @param _name	Textureの名前
	 * @return Textureへのポインタ, 存在しない場合: nullptr
	 */
	TexturePtr Material::GetTexture(const std::string& _name) const
	{
		if (auto it = textures_.find(_name); it != textures_.end()) {
			return it->second;
		}

		return nullptr;
	}
}
