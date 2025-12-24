#pragma once
/**
 * @file Texture.h
 * @brief テクスチャー
 */

 // ---------- インクルード ---------- //
#include <string>
#include <memory>
#include <wrl.h>
#include <d3d11.h>

namespace dx3d {
	namespace texture {
		/**
		 * @brief テクスチャークラス
		 */
		class Texture {
		public:
			//! @brief コンストラクタ
			Texture(
				Microsoft::WRL::ComPtr<ID3D11Texture2D> _texture,
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv) noexcept
				: texture_(_texture)
				, srv_(_srv) {}

			/**
			 * @brief SRVを取得
			 * @return SRVポインタ
			 */
			ID3D11ShaderResourceView* GetSRV() const noexcept { return srv_.Get(); }

		private:
			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_{};
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_{};
		};
	}
}