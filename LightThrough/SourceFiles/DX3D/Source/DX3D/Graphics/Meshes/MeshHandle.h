#pragma once
/**
 * @file MeshHandle.h
 * @brief メッシュのハンドル
 * @author Arima Keita
 * @date 2025-10-16
 */

 // ---------- インクルード ---------- //
#include <cstdint>
#include <Game/Serialization/ComponentReflection.h>

namespace dx3d {
	/**
	 * @brief メッシュのハンドル
	 *
	 * メッシュを識別するためのハンドル
	 * MeshRegistryでメッシュを登録・管理する際に使用する
	 */
	struct MeshHandle {
		uint32_t id = 0;
		bool IsValid() const { return id != 0; }
		auto operator<=>(const MeshHandle&) const = default;
	};
}

ECS_REFLECT_BEGIN(dx3d::MeshHandle)
ECS_REFLECT_FIELD(id)
ECS_REFLECT_END()