/**
 * @file ComponentManager.cpp
 * @brief コンポーネントマネージャ
 * @author Arima Keita
 * @date 2025-08-17
 */

 // ---------- インクルード ---------- // 
#include <Game/ECS/ComponentManager.h>

namespace ecs {
	/**
	 * @brief Componentの追加（voidポインタ版）
	 * @param _e 追加先のEntity
	 * @param _type 追加するComponentの種類
	 * @param _data 追加するComponentのデータへのポインタ
	 */
	void ComponentManager::AddComponent(Entity _e, ComponentType _type, const void* _data)
	{
		component_arrays_by_type_[_type]->AddRaw(_e, _data);
	}

	/**
	 * @brief Componentの削除
	 * @param _e 削除先のEntity
	 * @param _type 削除するComponentの種類
	 */
	void ComponentManager::RemoveComponent(Entity _e, ComponentType _type)
	{
		component_arrays_by_type_[_type]->Remove(_e);
	}
	/**
	 * @brief Entityが破棄された際に呼び出す
	 */
	void ComponentManager::EntityDestroyed(Entity _e)
	{
		for (auto& pair : this->component_arrays_) {
			pair.second->EntityDestroyed(_e);
		}
	}
}