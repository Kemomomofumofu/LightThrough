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
static bool IsCursorInClient(HWND _hwnd)
{
	POINT p{};
	::GetCursorPos(&p);
	POINT client = p;
	::ScreenToClient(_hwnd, &client);
	RECT rc{};
	::GetClientRect(_hwnd, &rc);

	return ::PtInRect(&rc, client);
}

/**
 * @brief ImGuiの入力キャプチャを適用
 *
 * ImGuiの操作中はゲームに対する入力を無効化する
 *
 * @param _inputSys	インプットシステム
 */
static void ApplyImGuiInputCapture(input::InputSystem& _inputSys)
{
	ImGuiIO& io = ImGui::GetIO();
	// UI 操作中
	if (io.WantCaptureMouse) {
		_inputSys.SetInputEnabled(false);
	}
	// UI 操作外
	else {
		_inputSys.SetInputEnabled(true);
	}
}


static LRESULT CALLBACK WindowProcedure(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(_hwnd, _msg, _wparam, _lparam)) { return 0; }	// ImGuiがイベントを消費したらスキップ

	auto& inputSystem = input::InputSystem::Get();

	switch (_msg)
	{
	// フォーカスが当たった
	case WM_SETFOCUS:
	{
		inputSystem.SetFocus(true);
		ApplyImGuiInputCapture(inputSystem);

		return 0;
	}
	// フォーカスが外れた
	case WM_KILLFOCUS:
	{
		if (inputSystem.IsMouseLocked()) {
			inputSystem.LockMouse(false);
		}
		inputSystem.SetFocus(false);
		::ClipCursor(nullptr);
		::ShowCursor(TRUE);

		return 0;;
	}
	// RawInput
	case WM_INPUT:
	{
		inputSystem.OnRawInput(_lparam);

		return 0;
	}
	// マウスホイール
	case WM_MOUSEWHEEL:
	{
		// [ToDo] RawInput無効時の場合でも入力取りたいならここに書く
		return 0;
	}
	// 左クリック
	case WM_LBUTTONDOWN:
	{
		ApplyImGuiInputCapture(inputSystem);

		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse) {
			// UI 操作中
			return 0;
		}

		// ゲーム内操作開始トリガー：クリックでロック
		if (!inputSystem.IsMouseLocked() && IsCursorInClient(_hwnd)) {
			inputSystem.LockMouse(true);
		}
		inputSystem.SetFocus(true);
		return 0;
	}
	// 右クリック
	case WM_RBUTTONDOWN:
	{
		return 0;
	}
	// キー
	case WM_KEYDOWN:
	{
		// ESC: ロック解除
		if (_wparam == VK_ESCAPE) {
			if (inputSystem.IsMouseLocked()) {
				inputSystem.LockMouse(false);	// マウスロック解除
				::ClipCursor(nullptr);
				::ShowCursor(TRUE);
			}
			return 0;
		}

		// F1: ロックトグル
		if (_wparam == VK_F1) {
			if (inputSystem.IsMouseLocked()) {
				inputSystem.LockMouse(false);
				::ClipCursor(nullptr);
				::ShowCursor(TRUE);
			}
			else {
				inputSystem.LockMouse(true);
			}
			return 0;
		}
		return 0;
	}
	// ウィンドウが閉じられた
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;;
	}
	default:
		break;
	}

	return ::DefWindowProc(_hwnd, _msg, _wparam, _lparam);
}

dx3d::Window::Window(const WindowDesc& _desc) : Base(_desc.base), size_(_desc.size)
{
	HINSTANCE hInstance = ::GetModuleHandle(nullptr);

	auto registerWindowClassFunction = [hInstance]() {
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = &WindowProcedure;
		wc.hInstance = hInstance;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon == LoadIcon(NULL, IDI_APPLICATION);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszClassName = "LIGHT THROUGH";
		return RegisterClassEx(&wc);
		};

	static const auto windowClassId = std::invoke(registerWindowClassFunction);

	if (!windowClassId) {
		DX3DLogThrowError("RegisterClassEx を 失敗しました");
	}

	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	RECT rc{ 0, 0, size_.width, size_.height };
	::AdjustWindowRect(&rc, style, FALSE);

	handle_ = ::CreateWindowEx(
		NULL,
		MAKEINTATOM(windowClassId),
		"Light Through",
		style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!handle_) {
		DX3DLogThrowError("CreateWindowEx を 失敗しました")
	}

	::ShowWindow(static_cast<HWND>(handle_), SW_SHOW);
	::UpdateWindow(static_cast<HWND>(handle_));
}


dx3d::Window::~Window() {
	::DestroyWindow(static_cast<HWND>(handle_));
}