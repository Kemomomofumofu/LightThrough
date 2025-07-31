#pragma once

/**
 * @file GraphicsLogUtils.h
 * @brief グラフィック用のログ出力メソッド
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Core/Logger.h>
#include <d3d11.h>


namespace dx3d {
	namespace GraphicsLogUtils {
		inline void CheckShaderCompile(Logger& _logger, HRESULT _hr, ID3DBlob* _errorBlob) {
			auto errorMsg = _errorBlob ? static_cast<const char*>(_errorBlob->GetBufferPointer()) : nullptr;

			if (FAILED(_hr)) {
				DX3DLogThrow(_logger, std::runtime_error, Logger::LogLevel::Error, errorMsg ? errorMsg : "Shader compilation を失敗した。");

				if (errorMsg) {
					DX3DLog(_logger, Logger::LogLevel::Warning, errorMsg);
				}
			}
		}
	}
}

#define DX3DGraphicsLogThrowOnFail(_hr, _message){\
			auto res = (_hr);\
			if(FAILED(res))\
				DX3DLogThrowError(_message);\
		}

#define DX3DGraphicsCheckShaderCompile(_hr, _errorBlob){\
			auto res = (_hr);\
			dx3d::GraphicsLogUtils::CheckShaderCompile(GetLogger(), res, _errorBlob);\
		}