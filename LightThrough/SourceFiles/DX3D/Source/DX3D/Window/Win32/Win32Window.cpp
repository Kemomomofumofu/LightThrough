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
#include <Game/InputSystem/InputSystem.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>


// ImGuiのWin32用イベントハンドラ
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
	if (ImGui_ImplWin32_WndProcHandler(_hwnd, _msg, _wparam, _lparam)) { return true; }	// ImGuiがイベントを消費したらスキップ


	// [ToDo] ImGui操作時はゲームに対する入力を無効化したい
	auto& inputSystem = input::InputSystem::Get();


	switch (_msg)
	{
		// フォーカスが当たった
	case WM_SETFOCUS:
	{

		break;
	}
	// フォーカスが外れた
	case WM_KILLFOCUS:
	{
		if (inputSystem.IsMouseLocked()) {
			inputSystem.LockMouse(false);
		}
		inputSystem.SetFocus(false);
		break;
	}
	case WM_LBUTTONDOWN:
	{
		inputSystem.SetFocus(true);

		// ImGuiがマウスをキャプチャしているならスキップ
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse) { break; }

		static bool wasPressed = false;
		bool isPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

		// クライアント領域内か
		if (IsCursorInClient(_hwnd) && !inputSystem.IsMouseLocked()) {
			// 押された瞬間か
			if (isPressed && !wasPressed) {
				inputSystem.LockMouse(true);	// マウスロック
			}
		}

		wasPressed = isPressed;
		break;
	}
	case WM_KEYDOWN:
	{
		// ESCキーが押された
		if (_wparam == VK_ESCAPE) {
			if (inputSystem.IsMouseLocked()) {
				inputSystem.LockMouse(false);	// マウスロック解除
			}
			inputSystem.SetFocus(false);
		}
		break;
	}
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

	handle_ = CreateWindowEx(NULL, MAKEINTATOM(windowClassId), "Light Through",
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