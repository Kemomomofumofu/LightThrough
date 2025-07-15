#pragma once
/**
 * @file Base.h
 * @brief ベースクラス
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Core/Common.h>


/**
 * @brief ベースクラス
 *
 * このクラスを継承するといい感じ
 */
namespace dx3d {
	class Base {
	public:
		explicit Base(const BaseDesc& _desc);
		virtual ~Base();
		virtual Logger& GetLogger() noexcept final;

	protected:
		Base(const Base&) = delete;
		Base(Base&&) = delete;
		Base& operator = (const Base&) = delete;
		Base& operator=(Base&&) = delete;

	protected:
		Logger& logger_;
	};
}

#define DX3DLogInfo(message)\
		DX3DLog(GetLogger(), Logger::LogLevel::Info, message)

#define DX3DLogWarning(message)\
		DX3DLog(GetLogger(), Logger::LogLevel::Warning, message)

#define DX3DLogError(message)\
		DX3DLog(GetLogger(), Logger::LogLevel::Error, message)

#define DX3DLogThrowError(message)\
		DX3DLogThrow(GetLogger(), std::runtime_error, Logger::LogLevel::Error, message)

#define DX3DLogThrowInvalidArg(message)\
		DX3DLogThrow(GetLogger(), std::invalid_argument, Logger::LogLevel::Error, message)