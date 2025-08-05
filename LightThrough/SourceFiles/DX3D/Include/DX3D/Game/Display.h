#pragma once

/**
 * @file Display.h
 * @brief スワップチェインを持っている
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Window/Window.h>

namespace dx3d {
	class Display final : public Window {
	public:
		explicit Display(const DisplayDesc& _desc);

		SwapChain& GetSwapChain() noexcept;

	private:
		SwapChainPtr swap_chain_{};
	};
}