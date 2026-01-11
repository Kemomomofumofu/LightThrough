#pragma once
/**
 * @file TextureLoader.h
 * @brief テクスチャを読み込むローダークラス
 */

 // ---------- インクルード ---------- //
#include <string>
#include <filesystem>
#include <memory>
#include <optional>
#include <DirectXTex.h>

namespace dx3d {
	namespace texture {
		// 前方宣言
		class Texture;

		/**
		 * @brief テクスチャーローダークラス
		 */
		class TextureLoader
		{
		public:
			explicit TextureLoader(ID3D11Device& _device)
				: device_(_device) {}

			/*
			 * @brief ファイルからテクスチャーを読み込む
			 * @param _path ファイルパス
			 * @param _srgb sRGBとして読み込むかどうか
			 * @return 読み込まれたテクスチャ、失敗した場合はstd::nullopt
			 */
			std::shared_ptr<Texture> Load(const std::filesystem::path&_path, bool _srgb) const;

		private:
			ID3D11Device& device_;

		};
	} // namespace texture
} // namespace dx3d