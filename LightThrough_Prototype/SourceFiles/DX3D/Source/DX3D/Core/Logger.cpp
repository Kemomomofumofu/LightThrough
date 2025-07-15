
/**
 * @file Logger.cpp
 * @brief ログを出力
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Core/Logger.h>
#include <iostream>


/**
 * @brief コンストラクタ
 * @param _logLevel ログのレベル
 */
dx3d::Logger::Logger(LogLevel _logLevel) : log_level_(_logLevel) {
	std::clog << "LightThrough | Prototype\n";
	std::clog << "------------------------\n";
}

dx3d::Logger::~Logger()
{
}

/**
 * @brief ログ出力
 * @param _level ログレベル
 * @param _message 出力する文字列
 */
void dx3d::Logger::Log(LogLevel _level, const char* _message)
{
	auto logLevelToString = [](LogLevel _level) {
		switch (_level)
		{
		case LogLevel::Info:	return "Info";
		case LogLevel::Warning:	return "Warning";
		case LogLevel::Error:	return "Error";
		default:				return "Unknown";
		}
		};

	if (_level > log_level_) { return; }
	std::clog << "[DX3D" << logLevelToString(_level) << "]" << _message << "\n";
}
