/**
 * @file Win32Window.cpp
 * @brief ウィンドウの初期化処理
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Window/Window.h>
#include <Windows.h>
#include <stdexcept>
#include <InputSystem/InputSystem.h>


/**
 * @brief カーソルがクライアント領域内か
 * @param _hwnd		ウィンドウハンドル
 * @return 領域内: true, 領域外: false
 */
static bool IsCursorInClient(HWND _hwnd) {
	POINT p{};
	::GetCursorPos(&p);
	POINT client = p;
	::ScreenToClient(_hwnd, &client);
	RECT rc{};
	::GetClientRect(_hwnd, &rc);
	
	return ::PtInRect(&rc, client);
}

static LRESULT CALLBACK WindowProcedure(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam) {
	switch (_msg)
	{
	// フォーカスが当たった
	case WM_SETFOCUS:

		break;
	// フォーカスが外れた
	case WM_KILLFOCUS:
		if (input::InputSystem::Get().IsMouseLocked()) {
			input::InputSystem::Get().LockMouse(false);
		}
		input::InputSystem::Get().SetFocus(false);
		break;
	case WM_LBUTTONDOWN:
		input::InputSystem::Get().SetFocus(true);

		// クライアント領域内であればマウスロック
		if (IsCursorInClient(_hwnd) && !input::InputSystem::Get().IsMouseLocked()) {
			input::InputSystem::Get().LockMouse(true);
		}
		break;
	// ウィンドウが閉じられた
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