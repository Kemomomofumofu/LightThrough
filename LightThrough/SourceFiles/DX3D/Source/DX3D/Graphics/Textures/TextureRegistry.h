#pragma once
/**
 * @file TextureRegistry.h
 */

 // ---------- インクルード ---------- //
#include <string>
#include <unordered_map>
#include <memory>
#include <DX3D/Core/Core.h>

struct ID3D11Device;
struct ID3D11ShaderResourceView;

namespace dx3d {
	class Texture;
	/**
	 * @brief 読み込まれたテクスチャをidで管理するレジストリクラス
	 * @details 現状UIのみでの使用を想定しているため、sRGBでの読み込みのみ実装
	 *
	 */
	class TextureRegistry
	{
	public:
		explicit TextureRegistry(ID3D11Device* _device);
		/**
		 * @brief テクスチャーを取得
		 * @param _path ファイルパス
		 * @return テクスチャーポインタ
		 *
		 * @details キャッシュに存在しない場合はロードしてキャッシュに保存する
		 */
		const TexturePtr Get(const std::string& _path);

	private:
		ID3D11Device* device_{};

		std::unordered_map<std::string, TexturePtr> cache_{}; // key -> Texture
		//std::unordered_map<std::string, std::string> key_to_path_{}; // key -> path // memo: 将来的にkeyでの管理に変更する場合用
	};
} // namespace dx3d
