#pragma once
/**
 * @file RasterizerState.h
 * @brief ラスタライザーステート
 * @author Arima Keita
 * @date 2025-09-04
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d {
	/**
	 * @brief ラスタライザーステートクラス
	 *
	 * 
	 */
	class RasterizerState final : public GraphicsResource {

	public:
		RasterizerState(const RasterizerStateDesc& _desc, const GraphicsResourceDesc& _gDesc);
	private:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rast_state_{};

		friend class DeviceContext;
	};
}