#pragma once
/**
 * @file ComponentManager.inl
 * @brief コンポーネントマネージャのテンプレート関数の定義
 * @author Arima Keita
 * @date 2025-08-16
 */


 // ---------- インクルード ---------- // 
#include <Game/ECS/Entity.h>
#include <Game/ECS/ComponentArray.h>
#include <Game/ECS/Coordinator.h>


namespace ecs {
	/**
	 * @brief Componentリストの登録
	 * @param <Com> 登録するComponentの種類
	 */
	template<typename Com>
	void ComponentManager::RegisterComponent()
	{
		// [ToDo] typeid(Com)との比較じゃなくて、type_index(typeid(Com))にしろとCopilotさんが言っていたのでメモ
		const std::type_index type = typeid(Com);
		assert(component_arrays_.find(type) == component_arrays_.end());

		// ComponentTypeを登録
		assert(next_component_type_ < MAX_COMPONENTS);
		ComponentType id = next_component_type_++;
		component_types_[type] = id;
		// Componentリストを登録
		component_arrays_[type] = std::make_unique<ComponentArray<Com>>();
		// ComponentTypeからComponentArrayを取得するための配列に登録
		component_arrays_by_type_[id] = component_arrays_[type].get();
	}

	/**
	 * @brief Componentの取得
	 * @param <Com> 取得するComponentの種類
	 * @param _e 取得先のComponent
	 * @return Component
	 */
	template<typename Com>
	Com* ComponentManager::GetComponent(Entity _e)
	{
		return GetComponentArray<Com>()->Get(_e);
	}

	/**
	* @brief Componentを持っているか
	* @param <Com> 確認するComponentの種類
	* @param _e 確認先のEntity
	* @return 有る: true, 無い: false
	*/
	template<typename Com>
	bool ComponentManager::HasComponent(Entity _e)
	{
		return GetComponentArray<Com>()->Has(_e);
	}

	/**
	 * @brief Componentタイプの取得
	 * @param <Com> 取得したいComponentの種類
	 * @return Componentタイプ
	 */
	template<typename Com>
	ComponentType ComponentManager::GetComponentType()
	{
		const std::type_index type = typeid(Com);
		assert(component_types_.find(type) != component_types_.end());
		return component_types_[type];
	}

	/**
	 * @brief Componentリストの取得
	 * @param <Com> 取得したいComponentの種類
	 * @return Componentリスト
	 */
	template<typename Com>
	ComponentArray<Com>* ComponentManager::GetComponentArray()
	{
		const std::type_index type = typeid(Com);
		assert(component_arrays_.find(type) != component_arrays_.end());
		return static_cast<ComponentArray<Com>*>(component_arrays_[type].get());
	}
}