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


namespace ecs {
	/**
	 * @brief システム群の取得
	 * @return 登録されたシステム群
	 */
	const std::unordered_map<std::type_index, std::shared_ptr<ISystem>>& SystemManager::GetAllSystems() const
	{
		return systems_;
	}

	//! システム群の取得（更新順）
	const std::vector<std::shared_ptr<ISystem>>& SystemManager::GetAllSystemsInOrder() const
	{
		return systems_in_order_;
	}

	//! @brief 全てのシステムを更新
	void SystemManager::UpdateAllSystems(float _dt)
	{
		for(auto system : systems_in_order_) {
			system->Update(_dt);
		}
	}

	//! @brief 登録されたSystemをすべて再アクティブ化
	void SystemManager::ReactivateAllSystems()
	{
		for (auto& pair : systems_) {
			pair.second->SetActive(true);
		}
	}


	//! エンティティのシグネチャが変わったときの処理
	void SystemManager::EntitySignatureChanged(Entity _e, Signature _eSignature)
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
	void SystemManager::EntityDestroyed(Entity _e)
	{
		for (auto const& pair : systems_) {

			pair.second->OnEntityDestroyed(_e);
			pair.second->entities_.erase(_e);
		}
	}
}