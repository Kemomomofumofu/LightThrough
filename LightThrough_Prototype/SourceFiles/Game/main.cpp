/**
 * @file main.cpp
 * @brief エイントリポイント
 * @author Arima Keita
 * @date 2025-06-25
 */


/*----- インクルード -----*/
#include <DX3D/All.h>


//--------------------------------------------------
// エントリポイント
//--------------------------------------------------
int main(void)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif	// DEBUG || _DEBUG

	try {
		dx3d::Game game({
				{1280, 720},					// ウィンドウサイズ
				dx3d::Logger::LogLevel::Info	// ログ
			});
		game.Run();	// ゲーム実行
	}
	catch (const std::runtime_error&) {
		return EXIT_FAILURE;
	}
	catch (const std::invalid_argument&) {
		return EXIT_FAILURE;
	}
	catch (const std::exception&) {
		return EXIT_FAILURE;
	}
	catch (...) {
		return EXIT_FAILURE;
	}



	return EXIT_SUCCESS;
}
