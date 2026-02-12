#pragma once
/**
 * @file TextureHandle.h
 * @brief テクスチャのハンドル
 */

#include <cstdint>
#include <Game/Serialization/ComponentReflection.h>

namespace dx3d {
    struct TextureHandle {
        uint32_t id = 0;
        bool IsValid() const { return id != 0; }
        auto operator<=>(const TextureHandle&) const = default;
    };
}

ECS_REFLECT_BEGIN(dx3d::TextureHandle)
ECS_REFLECT_FIELD(id)
ECS_REFLECT_END()
