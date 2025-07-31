/**
 * @file Display.h
 * @brief スワップチェインを持つ
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Game/Display.h>
#include <DX3D/Graphics/GraphicsDevice.h>


/**
 * @brief コンストラクタ
 * @param _desc ディスプレイの定義
 */
dx3d::Display::Display(const DisplayDesc& _desc) : Window(_desc.window) {
	// スワップチェーンの生成
	swap_chain_ = _desc.graphicsDevice.CreateSwapChain({ handle_, size_ });
}


/**
 * @brief スワップチェーンの取得
 * @return スワップチェーンの参照
 */
dx3d::SwapChain& dx3d::Display::GetSwapChain() noexcept {
	return *swap_chain_;
}


