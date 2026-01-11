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
#include <Game/ECS/Entity.h>

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
		virtual void EntityDestroyed(Entity _e) = 0;			// Entityが破棄された際に呼び出す
		virtual void Remove(Entity _e) = 0;						// Componentの削除
		virtual bool Has(Entity _e) const = 0;					// Componentを持っているか
		virtual void AddRaw(Entity _e, const void* _data) = 0;	// Componentの追加（voidポインタ版）
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
		Com& Get(Entity _e);
		//const std::vector<Com>& GetAllComponents() const;
		//const std::vector<Entity>& GetAllEntityIDs() const;
		

		void EntityDestroyed(Entity _e) override;
		void Remove(Entity _e) override;
		bool Has(Entity _e) const override;
		void AddRaw(Entity _e, const void* _src) override;

	private:
		std::vector<Com> components_{};	// ComponentのVector配列
		std::vector<Entity> entity_IDs_{};	// Componentに対応するEntityのVector配列
		std::unordered_map<Entity, size_t> entity_to_index_{};	// EntityとIndexを対応させるためのMap
	};


}


#include <Game/ECS/ComponentArray.inl>