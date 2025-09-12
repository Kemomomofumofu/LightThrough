// ECSLogUtils.h
#pragma once

/**
 * @file ECSLogUtils.h
 * @brief ECS用のログ出力メソッド
 * @author Arima Keita
 * @date 2025-09-11
 */

 /*---------- インクルード ----------*/
#include <DX3D/Core/Logger.h>
#include <string>
#include <sstream>
#include <format>
#include <DX3D/Game/ECS/Entity.h>
#include <Game/Components/Transform.h>

namespace ecs {
    namespace ECSLogUtils {
        inline std::string ToString(const Entity& _e)
        {
            return std::format("Entity(id = {}, version = {})", _e.Index(), _e.Version());
        }

        inline std::string ToString(const Transform& _t)
        {
            return std::format("Transform(Pos =({}, {}, {}), Rotation = ({}, {}, {}), Scale = ({}, {}, {})",
                _t.position.x, _t.position.y, _t.position.z,
                _t.rotation.x, _t.rotation.y, _t.rotation.z,
                _t.scale.x, _t.scale.y, _t.scale.z
            );
        }

        //inline void LogMatrix(dx3d::Logger& _logger, const DirectX::XMMATRIX& _matrix, const char* _name)
        //{
        //    _logger.Log(dx3d::Logger::LogLevel::Info, std::format("{}:", _name).c_str());

        //    char buffer[256];
        //    snprintf(buffer, sizeof(buffer),
        //        "[%f %f %f %f]",
        //     
        //}
    }
}

#define ECSLogFInfo(fmt, ...)\
    DX3DLogF(GetLogger(), dx3d::Logger::LogLevel::Info, fmt, __VA_ARGS__)
#define ECSLogFWarning(fmt, ...)\
    DX3DLogF(GetLogger(), dx3d::Logger::LogLevel::Warning, fmt, __VA_ARGS__)
#define ECSLogFError(fmt, ...)\
    DX3DLogF(GetLogger(), dx3d::Logger::LogLevel::Error, fmt, __VA_ARGS__)

#define ECSLogEntity(e)\
    DX3DLog(GetLogger(), dx3d::Logger::LogLevel::Info, ecs::ECSLogUtils::ToString(e).c_str())
#define ECSLogTransform(t)\
    DX3DLog(GetLogger(), dx3d::Logger::LogLevel::Info, ecs::ECSLogUtils::ToString(t).c_str())

