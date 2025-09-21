#pragma once
/**
 * @file Entity.h
 * @brief エンティティ
 * @author Arima Keita
 * @date 2025-08-06
 */

 // ---------- インクルード ---------- // 
#include <cstdint>
#include <compare>
#include <Game/ECS/ECSUtils.h>

namespace ecs {

	/**
	 * @brief エンティティ
	 *
	 * idはIndexとVersionを内包している
	 */
	struct Entity {
		uint32_t id_ = 0;

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


		// Indexを比較して、同じならバージョンを比較する
		std::strong_ordering operator<=>(const Entity& _other) const noexcept {
			if (Index() != _other.Index()){	return Index() <=> _other.Index(); }
			return Version() <=> _other.Version();
		}
	};
}

namespace std {
	// Entityのハッシュ関数の特殊化
	template <>
	struct hash<ecs::Entity> {
		std::size_t operator()(const ecs::Entity& _e) const noexcept {
			return std::hash<uint32_t>()(_e.id_);
		}
	};
}