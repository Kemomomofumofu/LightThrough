#pragma once
/**
 * @file GameLogUtils.h
 * @brief Game用のログ出力メソッド
 * @author Arima Keita
 * @date 2025-09-11
 */

 /*---------- インクルード ----------*/


#include <DX3D/Core/Logger.h>
#include <string>
#include <sstream>
#include <format>

#include <Game/ECS/Entity.h>
#include <Game/Components/Transform.h>

namespace game {
    namespace GameLogUtils {
        inline std::string ToString(const ecs::Entity& _e)
        {
            return std::format("Entity(id = {}, version = {})", _e.Index(), _e.Version());
        }
    }
}

#define GameLogFInfo(fmt, ...)\
    DX3DLogF(GetLogger(), dx3d::Logger::LogLevel::Info, fmt, __VA_ARGS__)
#define GameLogFWarning(fmt, ...)\
    DX3DLogF(GetLogger(), dx3d::Logger::LogLevel::Warning, fmt, __VA_ARGS__)
#define GameLogFError(fmt, ...)\
    DX3DLogF(GetLogger(), dx3d::Logger::LogLevel::Error, fmt, __VA_ARGS__)

#define GameLogInfo(message)\
    DX3DLog(GetLogger(), dx3d::Logger::LogLevel::Info, message)
#define GameLogWarning(message)\
    DX3DLog(GetLogger(), dx3d::Logger::LogLevel::Warning, message)
#define GameLogError(message)\
    DX3DLog(GetLogger(), dx3d::Logger::LogLevel::Error, message)

#define GameLogEntity(e)\
    DX3DLog(GetLogger(), dx3d::Logger::LogLevel::Info, log::GameLogUtils::ToString(e).c_str())
