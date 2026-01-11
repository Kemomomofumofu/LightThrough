#pragma once
/**
 * @file ComponentArray.inl
 * @brief コンポーネントリストのテンプレート関数の定義
 * @author Arima Keita
 * @date 2025-08-16
 */

// ---------- インクルード ---------- // 
#include <Game/ECS/Entity.h>
#include "ComponentArray.h"


namespace ecs {
	/**
	 * @brief Componentの追加
	 * @param _e			追加先のEntity
	 * @param _component	追加するComponentの参照
	 */
	template<typename Com>
	void ComponentArray<Com>::Insert(Entity _e, const Com& _component)
	{
		assert(entity_to_index_.find(_e) == entity_to_index_.end());
		entity_to_index_[_e] = components_.size();
		entity_IDs_.push_back(_e);
		components_.push_back(_component);
	}

	/**
	 * @brief Componentの削除
	 * @param _e		削除先のEntity
	 */
	template<typename Com>
	void ComponentArray<Com>::Remove(Entity _e)
	{
		auto it = entity_to_index_.find(_e);
		// 見つからなかったら
		if (it == entity_to_index_.end()) {
			assert(false && "[ComponentArray] RemoveでEntityが見つからなかった");
			return;
		}

		const size_t index = entity_to_index_[_e];
		const size_t lastIndex = components_.size() - 1;
		// もともと最後の要素ならSwapしない
		if (index != lastIndex)
		{
			// 削除する要素と最後の要素を入れ替える
			components_[index] = components_[lastIndex];
			entity_IDs_[index] = entity_IDs_[lastIndex];
			entity_to_index_[entity_IDs_[index]] = index;
		}

		components_.pop_back();
		entity_IDs_.pop_back();
		entity_to_index_.erase(_e);
	}

	/**
	 * @brief Componentの取得
	 * @param _e		取得先のEntity
	 * @return			取得したComponentの参照
	 */
	template<typename Com>
	Com& ComponentArray<Com>::Get(Entity _e)
	{
		assert(entity_to_index_.find(_e) != entity_to_index_.end());
		return components_[entity_to_index_[_e]];
	}

	/**
	 * @brief EntityがComponentを持っているか
	 * @param _e		確認対象のEntity
	 * @return 有る: true, 無い: false
	 */
	template<typename Com>
	bool ComponentArray<Com>::Has(Entity _e) const
	{
		return entity_to_index_.find(_e) != entity_to_index_.end();
	}

	/**
	 * @brief Entityが破棄された際に呼び出す
	 * @param _e		破棄されたEntity
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

	/**
	 * @brief 生ポインタからComponentを追加
	 * @param _e		追加先のEntity
	 * @param _src		追加するComponentの生ポインタ
	 */
	template<typename Com>
	inline void ecs::ComponentArray<Com>::AddRaw(Entity _e, const void* _src)
	{
		assert(_src != nullptr);
		Insert(_e, *static_cast<const Com*>(_src));
	}
}