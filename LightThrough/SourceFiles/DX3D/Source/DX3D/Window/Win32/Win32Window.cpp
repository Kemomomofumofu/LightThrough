/**
 * @file Win32Window.cpp
 * @brief 
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Window/Window.h>
#include <Windows.h>
#include <stdexcept>

static LRESULT CALLBACK WindowProcedure(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam) {
	switch (_msg)
	{
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(_hwnd, _msg, _wparam, _lparam);
	}
	return 0;
}

dx3d::Window::Window(const WindowDesc& _desc) : Base(_desc.base), size_(_desc.size) {
	auto registerWindowClassFunction = []() {
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.lpszClassName = "LIGHT THROUGH";
		wc.lpfnWndProc = &WindowProcedure;
		return RegisterClassEx(&wc);
		};

	static const auto windowClassId = std::invoke(registerWindowClassFunction);

	if (!windowClassId) {
		DX3DLogThrowError("RegisterClassEx を 失敗しました");
	}

	RECT rc{ 0, 0, size_.width, size_.height };
	AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, false);

	handle_ = CreateWindowEx(NULL, MAKEINTATOM(windowClassId), "Fabitami | Light Through",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, NULL, NULL);

	if (!handle_) {
		DX3DLogThrowError("CreateWindowEx を 失敗しました")
	}

	ShowWindow(static_cast<HWND>(handle_), SW_SHOW);
}


dx3d::Window::~Window() {
	DestroyWindow(static_cast<HWND>(handle_));
}