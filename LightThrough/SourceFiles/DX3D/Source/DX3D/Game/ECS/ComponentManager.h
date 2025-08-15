#pragma once
/**
 * @file ComponentManager.h
 * @brief コンポーネントマネージャ
 * @author Arima Keita
 * @date 2025-08-06
 */

 // ---------- インクルード ---------- //
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <cassert>
#include <DX3D/Game/ECS/ComponentArray.h>
#include <DX3D/Game/ECS/Entity.h>

namespace ecs {

	/**
	 * @brief コンポーネントマネージャ
	 *
	 * コンポーネントの管理を担当するクラス
	 * 各コンポーネントのリストを保持し、Entityに対してコンポーネントの追加、削除、取得を行う。
	 */
	class ComponentManager {
	public:
		template<typename Com>
		void RegisterComponent();	// Componentリストの登録
		template<typename Com>
		void AddComponent(Entity _e, const Com& _component);	// Componentの追加
		template<typename Com>
		void RemoveComponent(Entity _e);	// Componentの削除
		template<typename Com>
		Com& GetComponent(Entity _e);	// Componentの取得
		template<typename Com>
		bool HasComponent(Entity _e);	// Componentを持っているか

	private:
		std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> component_arrays_;	// ComponentのTypeidとComponentリストとのMap

		template<typename Com>
		ComponentArray<Com>* GetArray();	// Componentリストの取得
	};


	// ---------- 実装 ---------- // 
	/**
	 * @brief Componentリストの登録
	 * @param <Com> 登録するComponentの種類
	 */
	template<typename Com>
	inline void ComponentManager::RegisterComponent()
	{
		// [ToDo] typeid(Com)との比較じゃなくて、type_index(typeid(Com))にしろとcopilotさんが言っていたのでメモ
		const std::type_index type = typeid(Com);
		assert(entity_to_index_.find(type) == entity_to_index_.end());
		// Componentリストを登録
		component_arrays_[type] = std::make_unique<ComponentArray<Com>>();
	}

	/**
	 * @brief Componentの追加
	 * @param <Com> 追加するComponentの種類
	 * @param _e  追加先のEntity
	 * @param _component 追加するComponentの参照
	 */
	template<typename Com>
	inline void ComponentManager::AddComponent(Entity _e, const Com& _component)
	{
		GetArray<T>()->Insert(_e, _component);
	}

	/**
	 * @brief Componentの削除
	 * @param <Com> 削除するComponentの種類
	 * @param _e 削除先のEntity
	 */
	template<typename Com>
	inline void ComponentManager::RemoveComponent(Entity _e)
	{
		GetArray<Com>()->Remove(_e);
	}

	/**
	 * @brief Componentの取得
	 * @param <Com> 取得するComponentの種類
	 * @param _e 取得先のComponent
	 * @return Component
	 */
	template<typename Com>
	inline Com& ComponentManager::GetComponent(Entity _e)
	{
		return GetArray<Com>()->Get(_e);
	}

	/**
	* @brief Componentを持っているか
	* @param <Com> 確認するComponentの種類
	* @param _e 確認先のEntity
	* @return 有る: true, 無い: false
	*/
	template<typename Com>
	inline bool ComponentManager::HasComponent(Entity _e)
	{
		return GetArray<Com>()->Has(_e);
	}

	/**
	 * @brief Componentリストの取得
	 * @param <Com> 取得したいComponentの種類
	 * @return Componentリスト
	 */
	template<typename Com>
	inline ComponentArray<Com>* ComponentManager::GetArray()
	{
		const std::type_index type = typeid(Com);
		assert(entity_to_index_.find(type) == entity_to_index_.end());
		return static_cast<ComponentArray<Com>*>(component_arrays_[type].get());
	}

}
