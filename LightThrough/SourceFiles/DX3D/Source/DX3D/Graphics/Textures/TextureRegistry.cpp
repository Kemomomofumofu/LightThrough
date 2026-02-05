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

		//! @brief テクスチャーを読み込む
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
				DebugLogError("[TextureRegistry] テクスチャ読み込みに失敗: " + _path + "\n");
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
				DebugLogError("[TextureRegistry] テクスチャ変換に失敗: " + _path + "\n");
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
				DebugLogError("[TextureRegistry] リソースの作成に失敗 " + _path + "\n");
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = meta.format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;	// memo: 現状UIでしか使わないのでミップマップは1で固定

			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
			hr = _device->CreateShaderResourceView(resource.Get(), &srvDesc, srv.GetAddressOf());
			if (FAILED(hr)) {
				DebugLogError("[TextureRegistry] シェーダーリソースビューの作成に失敗 " + _path + "\n");
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

	//! @brief テクスチャーを取得
	const TexturePtr TextureRegistry::Get(const std::string& _path)
	{
		// キャッシュヒット
		if (auto it = cache_.find(_path); it != cache_.end()) { return it->second; }

		// 存在しない場合はロードしてキャッシュに保存
		auto texture = LoadTextureSRGB(device_, _path);
		if (!texture) { return nullptr; }

		cache_.emplace(_path, texture);

		return texture;
	}
} // namespace dx3d
