#pragma once
/**
 * @file Texture.h
 * @brief テクスチャークラス
 * @author Arima Keita
 * @date 2025-09-27
 */

 // ---------- インクルード ---------- //
#include <string>
#include <memory>
#include <wrl.h>
#include <d3d11.h>

namespace dx3d {
	/**
	 * @brief テクスチャークラス
	 *
	 * 2Dテクスチャを扱うクラス
	 * [ToDo] まだ未実装
	 */
	class Texture {
	public:
		Texture(const std::string& _path) {};
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSRV() const {};
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_{};
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_{};
	};
}