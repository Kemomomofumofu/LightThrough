#pragma once
/**
 * @file Texture.h
 */

 // ---------- インクルード ---------- //
#include <wrl.h>
#include <d3d11.h>


namespace dx3d {
	/**
	 * @brief テクスチャークラス
	 */
	class Texture {
	public:
		//Texture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv, uint32_t _width, uint32_t _height)
		//	: srv_(_srv), width_(_width), height_(_height)
		//{
		//}

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_{};
		uint32_t width_{};
		uint32_t height_{};
	};
}



