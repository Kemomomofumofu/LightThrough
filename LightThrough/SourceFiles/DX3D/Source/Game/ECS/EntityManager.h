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
#include <Game/ECS/Entity.h>
#include <Game/ECS/ECSUtils.h>


namespace ecs {
	/**
	 * @brief エンティティマネージャ
	 *
	 * Entityの生成、破棄、有効か無効かの確認、管理をする
	 */
	class EntityManager {
	public:
		EntityManager();

		Entity Create();				// Entityの生成
		void Destroy(Entity _entity);	// Entityの破棄
		bool IsValid(Entity _entity) const;	// Entityが有効かどうかを確認

		void SetSignature(Entity _entity, const Signature& _signature);	// EntityのSignatureを設定
		Signature GetSignature(Entity _entity) const;				// EntityのSignatureを取得

		std::vector<Entity> GetAllEntities();	// 登録されている全てのEntityを取得
		std::size_t RegisterEntityCount() const { return versions_.size(); }	// 登録されているEntityの数を取得

	private:
		void EnsureCapacityForIndex(uint32_t _index);	// 指定したIndexのために必要な容量を確保する
	private:
		std::vector<uint32_t> versions_{};	// versions_[index] => version
		std::queue<uint32_t> free_index_{};		// 再利用可能なID
		uint32_t next_index_ = 0;	// 次のIndex (0はNull扱い)
		std::vector<Signature> signatures_{};	// EntityのSignature
	};
}


