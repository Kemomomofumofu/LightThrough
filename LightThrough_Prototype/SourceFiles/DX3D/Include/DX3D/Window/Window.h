/**
 * @file Window.h
 * @brief ウィンドウヘッダー
 * @author Arima Keita
 * @date 2025-06-25
 */

#pragma once

#include <DX3D/Core/Base.h>
#include <DX3D/Core/Common.h>


 /**
  * @brief ウィンドウクラス
  *
  * ウィンドウのハンドルや、サイズを保持するクラス
  */
namespace dx3d {
	class Window : public Base {
	public:
		explicit Window(const WindowDesc& _desc);
		virtual ~Window() override;

	protected:
		void* handle_{};
		Rect size_{};
	};
}
