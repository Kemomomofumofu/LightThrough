#pragma once
/**
 * @file SystemManager.inl
 * @brief システム達を管理するクラスのテンプレート関数の定義
 * @author Arima Keita
 * @date 2025-08-16
 */

// ---------- インクルード ---------- // 
#include <DX3D/Game/ECS/ISystem.h>
#include <DX3D/Game/ECS/ECSUtils.h>


namespace ecs {
	template<typename T>
	std::shared_ptr<T> SystemManager::RegisterSystem()
	{
		std::type_index type = typeid(T);
		assert(systems_.find(type) == systems_.end());
		auto system = std::make_shared<T>();
		systems_[type] = system;
		return system;
	}


	template<typename T>
	inline void SystemManager::SetSignature(Signature _signature)
	{
		std::type_index type = typeid(T);
		assert(systems_.find(type) != systems_.end());
		signature_[type] = _signature;
	}

	template<typename T>
	std::shared_ptr<T> SystemManager::GetSystem()
	{
		std::type_index type = typeid(T);
		assert(systems_.find(type) != systems_.end());
		return std::static_pointer_cast<T>(systems_[type]);
	}
}