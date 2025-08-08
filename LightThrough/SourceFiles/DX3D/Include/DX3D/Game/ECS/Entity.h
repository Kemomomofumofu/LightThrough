#pragma once
/**
 * @file Entity.h
 * @brief エンティティ
 * @author Arima Keita
 * @date 2025-08-06
 */

 // ---------- インクルード ---------- // 
#include <cstdint>
#include <DX3D/Game/ECS/ECSUtils.h>

namespace ecs {

	/**
	 * @brief エンティティ
	 *
	 * idはindexとversionを内包している
	 */
	struct Entity {
		std::uint32_t id_ = 0;

		Entity() = default;
		explicit Entity(uint32_t _id) : id_(_id) {}

		uint32_t Index() const { return ecs::GetIndex(id_); }
		uint32_t Version() const { return ecs::GetVersion(id_); }

		bool IsInitialized() const { return id_ != 0; }	// NullCheck用

		bool operator==(const Entity& _other) const {
			return id_ == _other.id_;
		}
		bool operator!=(const Entity& _other) const {
			return id_ != _other.id_;
		}
	};
}