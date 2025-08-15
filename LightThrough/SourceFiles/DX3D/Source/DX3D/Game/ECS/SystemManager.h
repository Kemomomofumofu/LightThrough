#pragma once
/**
 * @file SystemManager.h
 * @brief システム達を管理するクラス
 * @author Arima Keita
 * @date 2025-08-08
 */

// ---------- インクルード ---------- // 
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <cassert>
#include <DX3D/Game/ECS/ISystem.h>

namespace ecs {

	class SystemManager {
	public:
		template<typename T>
		std::shared_ptr<T> RegisterSystem();

		template<typename T> 
		void SetSignature(Signature _signature);	// システムのSignatureを設定

		void EntitySignatureChanged(Entity _e, Signature _eSignature);	// EntityのSignatureが変わった時に呼び出す
		void EntityDestroyed(Entity _e);	// Entityが破棄された時に呼び出す

	private:
		std::unordered_map<const char*, Signature> signature_;
		std::unordered_map<const char*, std::shared_ptr<ISystem>> systems_;
	};

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


}