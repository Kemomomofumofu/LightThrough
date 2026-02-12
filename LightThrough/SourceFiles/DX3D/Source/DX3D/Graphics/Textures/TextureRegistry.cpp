/**
 * @file TextureRegistry.cpp
 */

 // ---------- インクルード ---------- //
#include <DirectXTex.h>
#include <DX3D/Graphics/Textures/TextureRegistry.h>
#include <DX3D/Graphics/Textures/Texture.h>


#include <Debug/Debug.h>

namespace dx3d {

	namespace {
		// UTF-8文字列をワイド文字列に変換
		std::wstring ToWide(const std::string& _s)
		{
			if (_s.empty()) return{};

			int len = MultiByteToWideChar(CP_UTF8, 0, _s.c_str(), -1, nullptr, 0);
			if (len <= 0) {
				return {};
			}

			std::wstring w;
			w.resize(static_cast<size_t>(len - 1));
			MultiByteToWideChar(CP_UTF8, 0, _s.c_str(), -1, w.data(), len);

			return w;
		}

		// _strが_prefixで始まるかどうか
		bool StartsWith(const std::string& _s, const std::string& _prefix)
		{
			return _s.size() >= _prefix.size() && std::equal(_prefix.begin(), _prefix.end(), _s.begin());
		}
		// 文字列にスラッシュが含まれているかどうか
		bool HasAnySlash(const std::string& _s)
		{
			return _s.find('/') != std::string::npos || _s.find('\\') != std::string::npos;
		}

		TexturePtr LoadTextureSRGB(ID3D11Device* _device, const std::string& _path)
		{
			if (!_device) { return nullptr; }
			// 文字変換
			const std::wstring widePath = ToWide(_path);
			if (widePath.empty()) {
				DebugLogError("[TextureRegistry] ワイド文字変換に失敗\n");
				return nullptr;
			}

			// テクスチャ読み込み
			DirectX::ScratchImage image;
			HRESULT hr = DirectX::LoadFromWICFile(
				widePath.c_str(),
				DirectX::WIC_FLAGS_FORCE_SRGB,
				nullptr,
				image);

			if (FAILED(hr)) {
				DebugLogError("[TextureRegistry] テクスチャ読み込みに失敗: {}(hr=0x{:08X})\n ", _path, static_cast<uint32_t>(hr));
				return nullptr;
			}

			// sRGB変換
			DirectX::ScratchImage converted;
			const DXGI_FORMAT targetFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

			hr = DirectX::Convert(
				image.GetImages(),
				image.GetImageCount(),
				image.GetMetadata(),
				targetFormat,
				DirectX::TEX_FILTER_DEFAULT,
				DirectX::TEX_THRESHOLD_DEFAULT,
				converted);

			if (FAILED(hr)) {
				DebugLogError("[TextureRegistry] テクスチャ変換に失敗: {}\n", _path);
				return nullptr;
			}


			// リソース作成
			const auto& meta = converted.GetMetadata();
			Microsoft::WRL::ComPtr<ID3D11Resource> resource;
			hr = DirectX::CreateTexture(
				_device,
				converted.GetImages(),
				converted.GetImageCount(),
				meta,
				resource.GetAddressOf()
			);

			if (FAILED(hr)) {
				DebugLogError("[TextureRegistry] リソースの作成に失敗 {}\n", _path);
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = meta.format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;	// memo: 現状UIでしか使わないのでミップマップは1で固定

			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
			hr = _device->CreateShaderResourceView(resource.Get(), &srvDesc, srv.GetAddressOf());
			if (FAILED(hr)) {
				DebugLogError("[TextureRegistry] シェーダーリソースビューの作成に失敗 {}\n", _path);
				return nullptr;
			}

			auto tex = std::make_shared<Texture>();
			tex->srv_ = srv;
			tex->width_ = static_cast<uint32_t>(meta.width);
			tex->height_ = static_cast<uint32_t>(meta.height);

			return tex;
		}
	} // namespace anonymous


	TextureRegistry::TextureRegistry(ID3D11Device* _device)
		: device_(_device)
	{
	}

	//! @brief テクスチャーをロードしてハンドルを取得
	TextureHandle TextureRegistry::Load(const std::string& _path)
	{
		const std::string full = NormalizePath(_path);

		if (full.empty()) {
			DebugLogError("[TextureRegistry] 空のパスが指定されました。\n");
			return TextureHandle{};
		}

		if(auto it = path_to_handle_.find(full); it != path_to_handle_.end()) {return it->second;}


		DebugLogInfo("[TextureRegistry] テクスチャをロード: {}\n", full);


		auto texture = LoadTextureSRGB(device_, full);
		if (!texture) { return TextureHandle{}; }

		TextureHandle handle{};
		handle.id = next_id_++;
		path_to_handle_.emplace(full, handle);
		id_to_texture_.emplace(handle.id, texture);

		return handle;
	}

	//! @brief テクスチャーを取得
	const TexturePtr TextureRegistry::Get(const std::string& _path)
	{
		const std::string full = NormalizePath(_path);
		if(full.empty()){
			DebugLogError("[TextureRegistry] 空のパスが指定されました。\n");
			return nullptr;
		}

		// キャッシュヒット
		if (auto it = path_to_handle_.find(full); it != path_to_handle_.end()) { return Get(it->second); }

		// 存在しない場合はロードしてキャッシュに保存
		auto texture = LoadTextureSRGB(device_, full);
		if (!texture) { return nullptr; }

		TextureHandle handle{};
		handle.id = next_id_++;
		path_to_handle_.emplace(full, handle);
		id_to_texture_.emplace(handle.id, texture);

		return texture;
	}
	const TexturePtr TextureRegistry::Get(const TextureHandle& _handle)
	{
		if (auto it = id_to_texture_.find(_handle.id); it != id_to_texture_.end()) { return it->second; }

		DebugLogError("[TextureRegistry] 存在しないハンドルが指定されました。id: {}\n", _handle.id);
		return nullptr;
	}

	// ファイルパスの正規化
	std::string TextureRegistry::NormalizePath(const std::string& _path) const
	{
		if (_path.empty()) { return _path; }

		// すでにベースパスが含まれている場合はそのまま返す
		if (!base_path_.empty() && StartsWith(_path, base_path_)) { return _path; }

		return base_path_ + _path;
	}
} // namespace dx3d
