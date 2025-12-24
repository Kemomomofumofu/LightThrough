/**
 * @file Debug.cpp
 * @brief デバッグ
 * @author Arima Keita
 * @date 2025-10-18
 */

 /*--------------- インクルード ---------------*/
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <fstream>
#include <string>
#include <filesystem>

#include <Debug/Debug.h>

namespace debug {

	// 静的メンバ変数の定義
	std::ofstream Debug::out_file_;
	bool Debug::console_allocated_ = false;
	HANDLE Debug::console_handle_ = nullptr;
	WORD Debug::default_console_attr_ = 0;
	std::mutex Debug::mutex_;


	/**
	 * @brief 初期化
	 * @param _showConsole コンソールを表示するか
	 */
	void Debug::Init(bool _showConsole) {
#if defined(DEBUG) || defined(_DEBUG)

		// デバッグコンソールを表示
		if (_showConsole && !console_allocated_)
		{
			AllocConsole();
			FILE* fp = nullptr;
			freopen_s(&fp, "CONOUT$", "w", stdout);
			freopen_s(&fp, "CONOUT$", "w", stderr);
			freopen_s(&fp, "CONIN$", "r", stdin);
			std::cout << "Debug Console\n";
			console_allocated_ = true;		// コンソールを確保した
			console_handle_ = GetStdHandle(STD_OUTPUT_HANDLE);	// コンソールハンドルを取得

			CONSOLE_SCREEN_BUFFER_INFO info{};
			if (console_handle_ && GetConsoleScreenBufferInfo(console_handle_, &info)) {
				default_console_attr_ = info.wAttributes;
			}
		}

		// ログディレクトリ作成
		try {
			std::filesystem::create_directories("DebugLog");
		}
		catch (...) {
			// 失敗しても続行（ファイル出力は以降のOpen結果で判断）
		}

		// ログを書き出すファイルの初期化
		out_file_.open("DebugLog/DebugLog.txt", std::ios::out | std::ios::trunc);
		if (!out_file_.is_open()) {
			std::cout << "Failed to open DebugLog/DebugLog.txt\n";
			OutputDebugStringA("Failed to open DebugLog/DebugLog.txt\n");
		}

#endif	// DEBUG || _DEBUG
	}


	/**
	 * @brief シャットダウン
	 * @param waitKey 終了時に止めるか
	 */
	void Debug::Shutdown(bool waitKey) {
#if defined(DEBUG) || defined(_DEBUG)

		if (console_allocated_) {
			if (waitKey) {
				std::cout << "Press Enter Key to Exit...";
				std::cin.get();	// キー入力待ち
			}
			// 必要に応じてコンソールを閉じる
			FreeConsole();
		}

		if (out_file_.is_open()) {
			out_file_.close();
		}

#endif	// DEBUG || _DEBUG
	}



	/**
	 * @brief タイムスタンプ取得
	 * @return 文字列化したタイムスタンプ
	 */
	std::string Debug::GetTimestamp() {
		auto now = std::chrono::system_clock::now();	// 現在時刻を取得
		auto in_time_t = std::chrono::system_clock::to_time_t(now);	// 時刻をTime_t型に変換
		std::tm timeInfo{};
		localtime_s(&timeInfo, &in_time_t);	// スレッドセーフにしなきゃエラー出る
		std::stringstream ss;
		ss << std::put_time(&timeInfo, "%Y-%m-%d %X");	// 文字列に変換
		return ss.str();
	}

	/**
	 * @brief ログレベルを文字列に変換
	 * @param _level ログレベル
	 * @return 文字列化したログレベル
	 */
	std::string_view Debug::LogLevelToString(LogLevel _level) {
		switch (_level)
		{
		case LogLevel::LOG_INFO:    return "INFO";
		case LogLevel::LOG_WARNING: return "WARNING";
		case LogLevel::LOG_ERROR:   return "ERROR";
		default:                    return "UNKNOWN";
		}
	}

	/**
	 * @brief コンソールの文字色を設定
	 * @param _level ログレベル
	 */
	void Debug::SetConsoleColor(LogLevel _level) {
		if (!console_handle_) {
			return;
		}

		switch (_level) {
		case LogLevel::LOG_INFO:
			SetConsoleTextAttribute(console_handle_, FOREGROUND_GREEN | FOREGROUND_INTENSITY);	// 緑
			break;
		case LogLevel::LOG_WARNING:
			SetConsoleTextAttribute(console_handle_, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);	// 黄
			break;
		case LogLevel::LOG_ERROR:
			SetConsoleTextAttribute(console_handle_, FOREGROUND_RED | FOREGROUND_INTENSITY);	// 赤
			break;
		default:
			if (default_console_attr_) {
				SetConsoleTextAttribute(console_handle_, default_console_attr_);
			}
			break;
		}
	}

	/**
	 * @brief 既定の色に戻す
	 */
	void Debug::ResetConsoleColor() {
		if (console_handle_ && default_console_attr_) {
			SetConsoleTextAttribute(console_handle_, default_console_attr_);
		}
	}

	/**
	 * @brief 生メッセージ
	 * @param _level ログレベル
	 * @param message 出力する文字列
	 */
	void Debug::Log(LogLevel _level, std::string_view message) {
#if defined(DEBUG) || defined(_DEBUG)
		Write(_level, message);
#else
		(void)_level; (void)message;
#endif
	}

	void Debug::Write(LogLevel level, std::string_view message) {
#if defined(DEBUG) || defined(_DEBUG)
		// フィルタ
		if (level < level_threshold_.load(std::memory_order_relaxed)) {
			return;
		}

		const std::string logMessage =
			GetTimestamp() + " [" + std::string(LogLevelToString(level)) + "] " +
			std::string(message);

		std::scoped_lock lk(mutex_);

		// コンソールに出力
		if (console_allocated_) {
			SetConsoleColor(level);
			std::cout << logMessage << std::endl;
			ResetConsoleColor();
		}

		// Visual Studio の出力ウィンドウにも出す
		{
			std::string dbg = logMessage + "\n";
			OutputDebugStringA(dbg.c_str());
		}

		// ファイルに出力
		if (out_file_.is_open()) {
			out_file_ << logMessage << std::endl;
			out_file_.flush();
		}
#else
		(void)level; (void)message;
#endif
	}
}