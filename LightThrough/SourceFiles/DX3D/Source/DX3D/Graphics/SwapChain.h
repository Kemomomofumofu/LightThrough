#pragma once
/**
 * @file SwapChain.h
 * @brief スワップチェイン
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/GraphicsResource.h>

namespace dx3d {
	class SwapChain final : public GraphicsResource{
	public:
		SwapChain(const SwapChainDesc& _desc, const GraphicsResourceDesc& _gDesc);
		Rect GetSize() const noexcept;

		void Present(bool _vsync = false);
		
	private:
		void ReloadBuffers();
	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_{};	// スワップチェイン
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_{};	// レンダーターゲットビュー
		Rect size_{};		// ウィンドウサイズ

		// [ToDo] いつか依存性を注入することでfriendをなくしたい...
		friend class DeviceContext;
	};
}

