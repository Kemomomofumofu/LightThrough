#pragma once
/**
 * @file TextureRegistry.h
 */

 // ---------- インクルード ---------- //
#include <string>
#include <unordered_map>
#include <memory>
#include <DX3D/Core/Core.h>
#include <DX3D/Graphics/Textures/TextureHandle.h>

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
		 * @brief テクスチャーをロードしてハンドルを取得
		 * @param _path ファイルパス
		 * @return テクスチャーハンドル
		 */
		TextureHandle Load(const std::string& _path);

		/**
		 * @brief テクスチャーを取得
		 * @param _path ファイルパス
		 * @return テクスチャーポインタ
		 *
		 * @details キャッシュに存在しない場合はロードしてキャッシュに保存する
		 */
		const TexturePtr Get(const std::string& _path);
		const TexturePtr Get(const TextureHandle& _handle);

	private:
		std::string NormalizePath(const std::string& _path) const;


	private:
		ID3D11Device* device_{};

		int next_id_ = 1; // 次に割り当てるID

		std::unordered_map<std::string, TextureHandle	> path_to_handle_{}; // key -> handle
		std::unordered_map<int, TexturePtr> id_to_texture_{}; // handle.id -> Texture

		std::string base_path_ = "Assets/Textures/"; // テクスチャのベースパス
	};
} // namespace dx3d
