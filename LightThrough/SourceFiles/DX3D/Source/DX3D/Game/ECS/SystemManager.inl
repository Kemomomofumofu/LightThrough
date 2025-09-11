#pragma once
/**
 * @file SystemManager.inl
 * @brief システム達を管理するクラスのテンプレート関数の定義
 * @author Arima Keita
 * @date 2025-08-16
 */

// ---------- インクルード ---------- // 
#include <DX3D/Game/ECS/SystemManager.h>
#include <DX3D/Game/ECS/ISystem.h>
#include <DX3D/Game/ECS/Coordinator.h>
#include <DX3D/Game/ECS/ECSUtils.h>


namespace ecs {
	template<typename Sys>
	std::shared_ptr<Sys> SystemManager::RegisterSystem(ecs::Coordinator& _ecs)
	{
		std::type_index type = typeid(Sys);
		assert(systems_.find(type) == systems_.end());
		auto system = std::make_shared<Sys>(_ecs);
		systems_[type] = system;
		return system;
	}


	template<typename Sys>
	inline void SystemManager::SetSignature(Signature _signature)
	{
		std::type_index type = typeid(Sys);
		assert(systems_.find(type) != systems_.end());
		signature_[type] = _signature;
	}

	template<typename Sys>
	std::shared_ptr<Sys> SystemManager::GetSystem()
	{
		std::type_index type = typeid(Sys);
		assert(systems_.find(type) != systems_.end());
		return std::static_pointer_cast<Sys>(systems_[type]);
	}
}