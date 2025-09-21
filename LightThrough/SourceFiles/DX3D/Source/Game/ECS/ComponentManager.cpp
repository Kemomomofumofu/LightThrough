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
	 * @brief Entityが破棄された際に呼び出す
	 */
	void ComponentManager::EntityDestroyed(Entity _e)
	{
		for (auto& pair : this->component_arrays_) {
			pair.second->EntityDestroyed(_e);
		}
	}
}