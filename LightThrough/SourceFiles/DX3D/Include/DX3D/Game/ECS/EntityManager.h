#pragma once
/**
 * @file EntityManager.h
 * @brief エンティティマネージャ
 * @author Arima Keita
 * @date 2025-08-08
 */

// ---------- インクルード ---------- // 
#include <cstdint>
#include <vector>
#include <queue>
#include <DX3D/Game/ECS/Entity.h>
#include <DX3D/Game/ECS/ECSUtils.h>


namespace ecs {
	/**
	 * @brief エンティティマネージャ
	 *
	 * エンティティの生成、破棄、有効か無効かの確認をする
	 */
	class EntityManager {
	public:
		Entity Create();
		void Destroy(Entity _entity);
		bool IsValid(Entity _entity) const;

	private:
		std::vector<uint32_t> versions_{};	// EntityのVersion
		std::queue<uint32_t> free_index_{};		// 再利用可能なID
		uint32_t next_index_ = 0;	// 次のIndex
	};
}