#pragma once
/**
 * @file TextureRegistry.h
 * @brief 読み込まれたテクスチャをidで管理するレジストリクラス
 */

 // ---------- インクルード ---------- //
#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include <DX3D/Graphics/GraphicsResource.h>
#include <DX3D/Graphics/Textures/Texture.h>
#include <DX3D/Graphics/Textures/TextureLoader.h>


namespace dx3d {
	namespace texture {

		/**
		 * @brief テクスチャーレジストリクラス
		 */
		class TextureRegistry
		{
		public:
			//! @brief コンストラクタ
			explicit TextureRegistry(ID3D11Device& _device);

			/**
			 * @brief テクスチャーを取得
			 * @param _path ファイルパス
			 * @param srgb sRGBとして読み込むかどうか
			 * @return テクスチャーポインタ
			 * 
			 * memo: キャッシュに存在しない場合はロードしてキャッシュに保存する
			 */
			const std::shared_ptr<Texture> Get(const std::wstring& _path, bool srgb = true);

		private:
			std::unique_ptr<TextureLoader> loader_{}; // テクスチャーローダー
			std::unordered_map<std::wstring, std::shared_ptr<Texture>> cache_; // テクスチャキャッシュ

		};

	} // namespace texture
} // namespace dx3d
