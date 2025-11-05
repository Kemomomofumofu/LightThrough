#pragma once
/**
 * @file Debug.h
 * @brief コンソールにLogを出力するためのクラス
 * @author Arima Keita
 * @date 2025-10-18
 */

 /*--------------- インクルード ---------------*/
#include <fstream>
#include <string>
#include <string_view>
#include <Windows.h>
#include <mutex>
#include <atomic>

namespace debug {

	class Debug {
	public:
		enum class LogLevel {
			LOG_INFO,
			LOG_WARNING,
			LOG_ERROR
		};

		static void Init(bool _showConsole);
		static void Shutdown(bool waitKey = false);

		// 既存シグネチャを保持（互換）
		template<typename... Args>
		static void Log(LogLevel _level, const std::string& _format, Args&&... _args);

		// 生メッセージをそのまま出す場合
		static void Log(LogLevel _level, std::string_view message);

		// ログレベルのしきい値設定（未満は棄却）
		static void SetLevel(LogLevel level) noexcept { level_threshold_ = level; }

	private:
		static std::string GetTimestamp();							// タイムスタンプを取得
		static std::string_view LogLevelToString(LogLevel _level);	// ログレベルを文字列に変換
		static void SetConsoleColor(LogLevel _level);				// コンソールの文字色を設定
		static void ResetConsoleColor();							// 既定の色に戻す
		static void Write(LogLevel level, std::string_view message);// 実出力（スレッド安全）

		static std::ofstream out_file_;			// 出力先ファイル
		static bool console_allocated_;			// コンソールを確保したかどうか
		static HANDLE console_handle_;			// コンソールハンドル
		static WORD default_console_attr_;		// 起動時のコンソール既定属性
		static std::mutex mutex_;				// 出力保護
		static inline std::atomic<LogLevel> level_threshold_ = LogLevel::LOG_INFO; // しきい値
	};

	template<typename... Args>
	void Debug::Log(LogLevel _level, const std::string& _format, Args&&... _args) {
#if defined(DEBUG) || defined(_DEBUG)
		try {
			std::string message = std::vformat(_format, std::make_format_args(_args...));
			Write(_level, message);
		}
		catch (const std::format_error& e) {
			Write(LogLevel::LOG_ERROR, std::string("[format_error] ") + e.what());
		}
#else
		(void)_level; (void)_format; (void)sizeof...(_args);
#endif
	}
}

#define DebugLogInfo(fmt, ...) \
	debug::Debug::Log(debug::Debug::LogLevel::LOG_INFO, fmt, ##__VA_ARGS__)
#define DebugLogWarning(fmt, ...) \
	debug::Debug::Log(debug::Debug::LogLevel::LOG_WARNING, fmt, ##__VA_ARGS__)
#define DebugLogError(fmt, ...) \
	debug::Debug::Log(debug::Debug::LogLevel::LOG_ERROR, fmt, ##__VA_ARGS__)
