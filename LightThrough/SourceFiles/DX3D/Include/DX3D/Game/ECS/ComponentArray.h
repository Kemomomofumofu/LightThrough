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
	template<typename Com>
	class ComponentArray : public IComponentArray {
	public:
		void Insert(Entity _e, const Com& _component);
		void Remove(Entity _e);
		Com& Get(Entity _e);

		bool Has(Entity _e) const;
		void EntityDestroyed(Entity _e) override;

	private:
		std::vector<Com> components_{};	// ComponentのVector配列
		std::unordered_map<Entity, size_t> entity_to_index_{};	// EntityとIndexを対応させるためのMap
	};


}


#include <DX3D/Game/ECS/ComponentArray.inl>