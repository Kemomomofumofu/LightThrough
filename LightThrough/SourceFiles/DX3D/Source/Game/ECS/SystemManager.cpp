/**
 * @file SystemManager.cpp
 * @brief システムマネージャメンバ関数の定義
 * @author Arima Keita
 * @date 2025-08-16
 */

 // ---------- インクルード ---------- // 
#include <Game/ECS/SystemManager.h>
#include <Game/ECS/Entity.h>
#include <Game/ECS/ECSUtils.h>
#include <Game/ECS/ISystem.h>


/**
 * @brief システム群の取得
 * @return 登録されたシステム群
 */
const std::unordered_map<std::type_index, std::shared_ptr<ecs::ISystem>>& ecs::SystemManager::GetAllSystems() const
{
	return systems_;
}

//! システム群の取得（更新順）
const std::vector<std::shared_ptr<ecs::ISystem>>& ecs::SystemManager::GetAllSystemsInOrder() const
{
	return systems_in_order_;
}

//! エンティティのシグネチャが変わったときの処理
void ecs::SystemManager::EntitySignatureChanged(Entity _e, Signature _eSignature)
{
	for (auto const& pair : systems_) {
		auto const& type = pair.first;
		auto const& system = pair.second;
		auto const& sysSig = signature_[type];

		if ((_eSignature & sysSig) == sysSig) {
			system->entities_.insert(_e);
		}
		else {
			system->entities_.erase(_e);
		}
	}
}

//! エンティティが破棄されたときの処理
void ecs::SystemManager::EntityDestroyed(Entity _e)
{
	for (auto const& pair : systems_) {

		pair.second->OnEntityDestroyed(_e);
		pair.second->entities_.erase(_e);
	}
}