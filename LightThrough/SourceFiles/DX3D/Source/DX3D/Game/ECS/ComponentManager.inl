#pragma once
/**
 * @file ComponentManager.inl
 * @brief コンポーネントマネージャのテンプレート関数の定義
 * @author Arima Keita
 * @date 2025-08-16
 */


 // ---------- インクルード ---------- // 
#include <DX3D/Game/ECS/Entity.h>
#include <DX3D/Game/ECS/ComponentArray.h>
#include <DX3D/Game/ECS/Coordinator.h>


namespace ecs {
	/**
	 * @brief Componentリストの登録
	 * @param <Com> 登録するComponentの種類
	 */
	template<typename Com>
	void ComponentManager::RegisterComponent()
	{
		// [ToDo] typeid(Com)との比較じゃなくて、type_index(typeid(Com))にしろとcopilotさんが言っていたのでメモ
		const std::type_index type = typeid(Com);
		assert(component_arrays_.find(type) == component_arrays_.end());

		// ComponentTypeを登録
		assert(next_component_type_ < MAX_COMPONENTS);
		component_types_[type] = next_component_type_++;

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
	void ComponentManager::AddComponent(Entity _e, const Com& _component)
	{
		GetArray<Com>()->Insert(_e, _component);
	}

	/**
	 * @brief Componentの削除
	 * @param <Com> 削除するComponentの種類
	 * @param _e 削除先のEntity
	 */
	template<typename Com>
	void ComponentManager::RemoveComponent(Entity _e)
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
	Com& ComponentManager::GetComponent(Entity _e)
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
	bool ComponentManager::HasComponent(Entity _e)
	{
		return GetArray<Com>()->Has(_e);
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
	ComponentArray<Com>* ComponentManager::GetArray()
	{
		const std::type_index type = typeid(Com);
		assert(component_arrays_.find(type) != component_arrays_.end());
		return static_cast<ComponentArray<Com>*>(component_arrays_[type].get());
	}
}