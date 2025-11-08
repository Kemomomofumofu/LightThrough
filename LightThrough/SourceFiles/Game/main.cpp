/**
 * @file main.cpp
 * @brief エントリポイント
 * @author Arima Keita
 * @date 2025-06-25
 */


/*----- インクルード -----*/
#include <DX3D/All.h>
#include <Debug/Debug.h>

/**
 * エントリポイント
 */
int main(void)
{
	DebugLogInfo("[main] Entry on main");
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif	// DEBUG || _DEBUG

	try {
		dx3d::Game game({
				{1920, 1080},					// ウィンドウサイズ
				dx3d::Logger::LogLevel::Info	// ログ
			});
		game.Run();	// ゲーム実行
	}
	catch (const nlohmann::json::exception& _e) {
		DebugLogError("[main] json exception: {}", _e.what());
		return EXIT_FAILURE;
	}
	catch (const std::exception& _e) {
		DebugLogError("[main] std::exception: {}", _e.what());
		return EXIT_FAILURE;
	}
	catch (...) {
		DebugLogError("[main] unknown exception");
		return EXIT_FAILURE;
	}

	DebugLogInfo("[main] normal exit\n");
	return EXIT_SUCCESS;
}