#pragma once
/**
 * @file ComponentArray.h
 * @brief コンポーネントリスト
 * @author Arima Keita
 * @date 2025-08-15
 */

 // ---------- インクルード ---------- // 
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <cassert>
#include <DX3D/Game/ECS/Entity.h>

namespace ecs
{
	/**
	 * @brief コンポーネントリストのインターフェース
	 *
	 * コンポーネントリストが持つ基本機能を持っている
	 */
	class IComponentArray {
	public:
		virtual ~IComponentArray() = default;
		virtual void EntityDestroyed(Entity _e) = 0;	// Entityが破棄された際に呼び出す
	};

	/**
	 * @brief コンポーネントリスト
	 *
	 * 一種類のComponentに対して、一つのComponentArray()が存在する。
	 * どのEntityがComponentを持っているか？どんな値なのか？を保持している。
	 */
	template<typename T>
	class ComponentArray : public IComponentArray {
	public:
		void Insert(Entity _e, const T& _component);
		void Remove(Entity _e);
		T& Get(Entity _e);

		bool Has(Entity _e) const;
		void EntityDestroyed(Entity _e) override;

	private:
		std::vector<T> components_;	// ComponentのVector配列
		std::unordered_map<Entity, size_t> entity_to_index_;	// EntityとIndexを対応させるためのMap
	};


	// ComponentArrayの実装
	template<typename T>
	inline void ComponentArray<T>::Insert(Entity _e, const T& _component)
	{
		assert(entity_to_index_.find(_e) == entity_to_index_.end());
		entity_to_index_[_e] = components_.size();
		components_.push_back(_component);
	}

	template<typename T>
	inline void ComponentArray<T>::Remove(Entity _e)
	{
		assert(entity_to_index_.find(_e) == entity_to_index_.end());
		size_t index = entity_to_index_[_e];
		components_[index] = components.back();
		components_.pop_back();
		entity_to_index_.erase(_e);
	}

	template<typename T>
	inline T& ComponentArray<T>::Get(Entity _e)
	{
		assert(entity_to_index_.find(_e) == entity_to_index_.end());
		return components_[entity_to_index_[_e]];
	}

	/**
	 * @brief EntityがComponentを持っているか
	 * @param _e 確認対象のEntity
	 * @return 有る: true, 無い: false
	 */
	template<typename T>
	inline bool ComponentArray<T>::Has(Entity _e) const
	{
		return entity_to _indx_.find(_e) != entity_to_index_.end();
	}

	/**
	 * @brief Entityが破棄された際に呼び出す
	 * @param _e 破棄されたEntity
	 */
	template<typename T>
	inline void ComponentArray<T>::EntityDestroyed(Entity _e)
	{
		// あるなら
		if (Has(_e)) {
			// 削除
			Remove(_e);
		}
	}
}