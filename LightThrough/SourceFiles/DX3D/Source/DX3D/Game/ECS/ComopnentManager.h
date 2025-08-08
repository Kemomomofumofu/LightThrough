#pragma once
/**
 * @file ComponentManager.h
 * @brief コンポーネントマネージャ
 * @author Arima Keita
 * @date 2025-08-06
 */

// ---------- インクルード ---------- // 
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <cassert>
#include <DX3D/Game/ECS/Entity.h>

namespace ecs {

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
		void Insert(Entity _e, const T& _component) {
			assert(entity_to_index_.find(_e) == entity_to_index_.end());
			entity_to_index_[_e] = components_.size();
			components_.push_back(_component);
		}

		void Remove(Entity _e) {
			assert(entity_to_index_.find(_e) == entity_to_index_.end());
			size_t index = entity_to_index_[_e];
			components_[index] = components.back();
			components_.pop_back();
			entity_to_index_.erase(_e);
		}

		T& Get(Entity _e) {
			assert(entity_to_index_.find(_e) == entity_to_index_.end());
			return components_[entity_to_index_[_e]];
		}


		/**
		 * @brief EntityがComponentを持っているか
		 * @param _e 確認対象のEntity
		 * @return 有る: true, 無い: false
		 */
		bool Has(Entity _e) const {
			return entity_to _indx_.find(_e) != entity_to_index_.end();
		}


		/**
		 * @brief Entityが破棄された際に呼び出す
		 * @param _e 破棄されたEntity
		 */
		void EntityDestroyed(Entity _e) override {
			// あるなら
			if (Has(_e)) {
				// 削除
				Remove(_e);
			}
		}

	private:
		std::vector<T> components_;	// ComponentのVector配列
		std::unordered_map<Entity, size_t> entity_to_index_;	// EntityとIndexを対応させるためのMap
	};

	class ComponentManager {
	public:

		/**
		 * @brief Componentリストの登録
		 * @param <T> 登録するComponentの種類
		 */
		template<typename T>
		void RegisterComponent() {
			const std::type_index type = typeid(T);
			assert(entity_to_index_.find(type) == entity_to_index_.end());
			// Componentリストを登録
			component_arrays_[type] = std::make_unique<ComponentArray<T>>();
		}

		/**
		 * @brief Componentの追加
		 * @param <T> 追加するComponentの種類
		 * @param _e  追加先のEntity
		 * @param _component 追加するComponentの参照
		 */
		template<typename T>
		void AddComponent(Entity _e, const T& _component) {
			GetArray<T>()->Insert(_e, _component);
		}

		/**
		 * @brief Componentの削除
		 * @param <T> 削除するComponentの種類
		 * @param _e 削除先のEntity
		 */
		template<typename T>
		void RemoveComponent(Entity _e) {
			GetArray<T>()->Remove(_e);
		}

		/**
		 * @brief Componentの取得
		 * @param <T> 取得するComponentの種類
		 * @param _e 取得先のComponent
		 * @return Component
		 */
		template<typename T>
		T& GetComponent(Entity _e) {
			return GetArray<T>()->Get(_e);
		}


		/**
		 * @brief Componentを持っているか
		 * @param <T> 確認するComponentの種類
		 * @param _e 確認先のEntity
		 * @return 有る: true, 無い: false
		 */
		template<typename T>
		bool HasComponent(Entity _e) {
			return GetArray<T>()->Has(_e);
		}

	private:
		std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> component_arrays_;	// ComponentのTypeidとComponentリストとのMap

		/**
		 * @brief Componentリストの取得
		 * @param <T> 取得したいComponentの種類
		 * @return Componentリスト
		 */
		template<typename T>
		ComponentArray<T>* GetArray() {
			const std::type_index type = typeid(T);
			assert(entity_to_index_.find(type) == entity_to_index_.end());
			return static_cast<ComponentArray<T>*>(component_arrays_[type].get());
		}
	};
}
