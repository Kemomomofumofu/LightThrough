/**
 * @file DebugUI.cpp
 * @brief ImGuiを使ったデバッグクラス
 * @author Arima Keita
 * @date 2025-09-15
 */

 // ---------- インクルード ---------- //
#include <Debug/DebugUI.h>
#include <vector>

namespace debug {
	std::vector<std::function<void(void)>> debug::DebugUI::debug_functions_{};

	/**
	 * @brief ImGuiの初期化
	 * @param _device D3D11デバイス
	 * @param _context D3D11デバイスコンテキスト
	 */
	void DebugUI::Init(ID3D11Device* _device, ID3D11DeviceContext* _context, void* _hwnd)
	{
		// ImGuiの初期化
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // キーボードによるナビゲーションを有効化
		// ImGuiのスタイル
		ImGui::StyleColorsDark();		// ダーク
		//ImGui::StyleColorsLight();	// ライト

		// ImGuiのバックエンド初期化
		ImGui_ImplWin32_Init(_hwnd);
		ImGui_ImplDX11_Init(_device, _context);
	}
	/**
	 * @brief デバッグ関数の登録
	 * @param _func 登録する関数
	 */
	void DebugUI::ResistDebugFunction(std::function<void(void)> _func)
	{
		debug_functions_.emplace_back(std::move(_func));
	}
	/**
	 * @brief デバッグUIの描画
	 */
	void DebugUI::Render()
	{
		ImGui_ImplDX11_CreateDeviceObjects();	// なぜかこれを明示的に呼び出さないとフォントが崩れる

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// デバッグ情報の表示
		ImGui::Begin("Debug Information");
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

		// 登録されたデバッグ関数の実行
		for (auto& func : debug_functions_) {
			func();
		}
		// 描画
		ImGui::ShowMetricsWindow();
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
	/**
	 * @brief デバッグUIの破棄
	 */
	void DebugUI::DisposeUI()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}
