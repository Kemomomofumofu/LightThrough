/**
 * @file TextureLoader.cpp
 */

 // ---------- インクルード ---------- // 
#include <wrl.h>
#include <DirectXTex.h>

#include <DX3D/Graphics/Textures/TextureLoader.h>
#include <DX3D/Graphics/Textures/Texture.h>

#include <Debug/Debug.h>


namespace dx3d {
	namespace texture {
		using namespace DirectX;
		using Microsoft::WRL::ComPtr;

		//! @brief テクスチャーを読み込む
		std::shared_ptr<Texture> TextureLoader::Load(const std::filesystem::path& _path, bool _srgb) const
		{

			ScratchImage image;
			HRESULT hr = S_OK;

			auto path_w = _path.wstring();
			const auto ext = std::filesystem::path(path_w).extension().wstring();

			// 読み込み
			if (ext == L".dds") {
				hr = LoadFromDDSFile(
					_path.c_str(),
					DDS_FLAGS_NONE,
					nullptr,
					image
				);
			}
			else {
				hr = LoadFromWICFile(
					_path.c_str(),
					WIC_FLAGS_FORCE_SRGB,
					nullptr,
					image
				);
			}
			if (FAILED(hr)) {
				DebugLogError("[TextureLoader] テクスチャ読み込み失敗: " + _path.string());
				return nullptr;
			}


			// sRGB 変換
			const Image* img = image.GetImage(0, 0, 0);
			DXGI_FORMAT format = img->format;

			if (_srgb) {
				format = MakeSRGB(format);
			}

			// Texture2D 作成
			ComPtr<ID3D11Resource> resource;
			hr = CreateTexture(
				&device_,
				image.GetImages(),
				image.GetImageCount(),
				image.GetMetadata(),
				&resource
			);
			if (FAILED(hr)) {
				DebugLogError("[TextureLoader] テクスチャリソースの作成失敗: " + _path.string());
				return nullptr;
			}

			ComPtr<ID3D11Texture2D> texture;
			hr = resource.As(&texture);
			if (FAILED(hr)) {
				DebugLogError("[TextureLoader] テクスチャリソースのTexture2D変換失敗: " + _path.string());
				return nullptr;
			}

			// SRV 作成
			D3D11_TEXTURE2D_DESC texDesc{};
			texture->GetDesc(&texDesc);

			CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = texDesc.MipLevels;

			ComPtr<ID3D11ShaderResourceView> srv;
			hr = device_.CreateShaderResourceView(
				texture.Get(),
				&srvDesc,
				&srv
			);
			if (FAILED(hr)) {
				DebugLogError("[TextureLoader] テクスチャSRVの作成失敗: " + _path.string());
				return nullptr;
			}

			// テクスチャーを返す
			return std::make_shared<Texture>(std::move(texture), std::move(srv));
		}
	} // namespace texture
} // namespace dx3d
