#pragma once
/**
 * @file GraphicsUtils.h
 * @brief グラフィック補助機能
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Core/Common.h>
#include <d3d11.h>
#include <bit>

#include <DX3D/Graphics/ShaderBinary.h>

/**
 * @brief GraphicsUtils
 *
 * レンダラーに対して楽になる補助的な処理を詰め込む空間
 */
namespace dx3d {
	namespace GraphicsUtils {
		inline const char* GetShaderModelTarget(ShaderBinary::Type _type) {
			switch (_type) {
			case ShaderBinary::Type::Vertex:	return "vs_5_0";
			case ShaderBinary::Type::Pixel:		return "ps_5_0";
			default:						return "";
			}
		}

		inline DXGI_FORMAT GetDXGIFormatFromMask(D3D_REGISTER_COMPONENT_TYPE _type, UINT _mask) {
			auto componentCount = std::popcount(_mask);
			if (componentCount < 1) {
				return DXGI_FORMAT_UNKNOWN;
			}

			constexpr DXGI_FORMAT formatTable[1][4] = {
				{
					DXGI_FORMAT_R32_FLOAT,
					DXGI_FORMAT_R32G32_FLOAT,
					DXGI_FORMAT_R32G32B32_FLOAT,
					DXGI_FORMAT_R32G32B32A32_FLOAT,
				}
			};

			auto typeIndex = 0u;
			switch (_type) {
			case D3D_REGISTER_COMPONENT_FLOAT32: typeIndex = 0u; break;
			default: return DXGI_FORMAT_UNKNOWN;
			}

			return formatTable[0][componentCount - 1];
		};
	}
}