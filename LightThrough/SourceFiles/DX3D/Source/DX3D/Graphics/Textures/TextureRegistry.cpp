/**
 * @file TextureRegistry.cpp
 */

 // ---------- インクルード ---------- //
#include <DX3D/Graphics/Textures/TextureRegistry.h>

#include <Debug/Debug.h>

namespace dx3d {
	namespace texture {
		//! @brief コンストラクタ
		TextureRegistry::TextureRegistry(ID3D11Device& _device)
			: loader_(std::make_unique<TextureLoader>(_device))
		{
		}

		//! @brief テクスチャーを取得
		const std::shared_ptr<Texture> TextureRegistry::Get(const std::wstring& _path, bool _srgb)
		{
			auto it = cache_.find(_path);
			// 存在する場合
			if (it != cache_.end()) {
				return it->second;
			}

			// 存在しない場合はロードしてキャッシュに保存
			auto tex = loader_->Load(_path, _srgb);
			if (tex) {
				cache_.emplace(_path, tex);
			}

			return tex;
		}
	} // namespace texture
} // namespace dx3d
