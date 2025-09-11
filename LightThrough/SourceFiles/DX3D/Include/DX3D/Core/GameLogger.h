#pragma once

/**
 * @file GameLogger.h
 * @brief Scene内で使うためのLogger
 * @author Arima Keita
 * @date 2025-09-11
 */

// ---------- インクルード ---------- // 
#include <unordered_map>
#include <fstream>

namespace ecs {
	class GameLogger final{
	public:
		// ログの緊急度
		enum class LogLevel {
			Error = 0,
			Warning,
			Info
		};

		enum class LogCategory {
			General = 0,
			Rendering,
			Physics,
			Audio,
			AI,
			ECS,
		};

		explicit GameLogger(LogLevel _logLevel = LogLevel::Info);

		template<typename ...Args>
		void LogFormat(LogLevel _level, LogCategory _category, const char* _format, Args... _args);

		bool EnableFileFogging(const char* _filename);

	private:
		std::unordered_map<LogCategory, bool> category_enabled_;
		std::ofstream log_file_;
	};

#define GAME_LOG_RENDER_INFO(_format, ...) \
	gameLogger.LogFormat(GameLogger::LogLevel::Info, GameLogger::LogCategory::Rendering, _format, __VA_ARGS__ )
}
