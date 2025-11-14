#pragma once
/**
 * @file DebugUI.h
 * @brief ImGuiを使ったデバッグUI
 * @author Arima Keita
 * @date 2025-09-15
 */

// ---------- インクルード ---------- // 
#include <d3d11.h>
#include <vector>
#include <functional>
#include <DX3D/Game/Game.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

namespace debug {
	class DebugUI {
#ifdef _DEBUG || DEBUG
		static std::vector<std::function<void(void)>> debug_functions_;
	public:
		static void Init(ID3D11Device* _device, ID3D11DeviceContext* _context, void* _hwnd);

		static void ResistDebugFunction(std::function<void(void)> _func);	//! デバッグ関数の登録
		static void Render();		//! デバッグUIの描画
		static void DisposeUI();	//! デバッグUIの破棄
#endif // _DEBUG || DEBUG
	};
}