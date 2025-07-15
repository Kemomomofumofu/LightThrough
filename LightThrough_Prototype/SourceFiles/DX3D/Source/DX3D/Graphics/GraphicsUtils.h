#pragma once
/**
 * @file GraphicsUtils.h
 * @brief グラフィック補助機能
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Core/Common.h>



/**
 * @brief GraphicsUtils
 *
 * レンダラーに対して楽になる補助的な処理を詰め込む空間
 */
namespace dx3d {
	namespace GraphicsUtils {
		inline const char* GetShaderModelTarget(ShaderType _type) {
			switch (_type) {
			case ShaderType::VertexShader:	return "vs_5_0";
			case ShaderType::PixelShader:	return "ps_5_0";
			default:						return "";
			}
		}
	}
}