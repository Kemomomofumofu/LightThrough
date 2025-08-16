#pragma once
/**
 * @file ComponentArray.inl
 * @brief コンポーネントリストのテンプレート関数の定義
 * @author Arima Keita
 * @date 2025-08-16
 */

// ---------- インクルード ---------- // 
#include <DX3D/Game/ECS/Entity.h>


namespace ecs {
	template<typename Com>
	void ComponentArray<Com>::Insert(Entity _e, const Com& _component)
	{
		assert(entity_to_index_.find(_e) == entity_to_index_.end());
		entity_to_index_[_e] = components_.size();
		components_.push_back(_component);
	}

	template<typename Com>
	void ComponentArray<Com>::Remove(Entity _e)
	{
		assert(entity_to_index_.find(_e) == entity_to_index_.end());
		size_t index = entity_to_index_[_e];
		components_[index] = components_.back();
		components_.pop_back();
		entity_to_index_.erase(_e);
	}

	template<typename Com>
	Com& ComponentArray<Com>::Get(Entity _e)
	{
		assert(entity_to_index_.find(_e) == entity_to_index_.end());
		return components_[entity_to_index_[_e]];
	}

	/**
	 * @brief EntityがComponentを持っているか
	 * @param _e 確認対象のEntity
	 * @return 有る: true, 無い: false
	 */
	template<typename Com>
	bool ComponentArray<Com>::Has(Entity _e) const
	{
		return entity_to_index_.find(_e) != entity_to_index_.end();
	}

	/**
	 * @brief Entityが破棄された際に呼び出す
	 * @param _e 破棄されたEntity
	 */
	template<typename Com>
	void ComponentArray<Com>::EntityDestroyed(Entity _e)
	{
		// あるなら
		if (Has(_e)) {
			// 削除
			Remove(_e);
		}
	}
}