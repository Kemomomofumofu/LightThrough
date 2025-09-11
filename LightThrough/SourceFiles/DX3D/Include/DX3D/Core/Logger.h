#pragma once
/**
 * @file Logger.h
 * @brief ログを出力
 * @author Arima Keita
 * @date 2025-06-25
 */

// ---------- インクルード ---------- // 
#include <format>
#include <iostream>
#include <string_view>

 /**
  * @brief Logger Class
  *
  * ログ出力するためのクラス
  */
namespace dx3d {
	class Logger final {
	public:
		// ログの緊急度
		enum class LogLevel {
			Error = 0,
			Warning,
			Info
		};

		explicit Logger(LogLevel _logLevel = LogLevel::Error);
		~Logger();

		void Log(LogLevel _level, const char* _message);

		template <typename... Args>
		void LogF(LogLevel  _level, std::string_view _fmt, Args&&... _args)
		{
			if (_level > log_level_) { return; }

			std::string msg = std::vformat(_fmt, std::make_format_args(_args...));
			Log(_level, msg.c_str());
		}


	protected:
		Logger(const Logger&) = delete;
		Logger(Logger&&) = delete;
		Logger& operator = (const Logger&) = delete;
		Logger& operator=(Logger&&) = delete;

	private :
		LogLevel log_level_ = LogLevel::Error;
	};
}

#define DX3DLog(logger, type, message)\
	logger.Log((type), message)

#define DX3DLogThrow(logger, exception, type, message)\
	{\
		DX3DLog(logger, type, message);\
		throw exception(message);\
	}

#define DX3DLogF(logger, type, fmt, ...)\
	logger.LogF((type), (fmt), ##__VA_ARGS__)