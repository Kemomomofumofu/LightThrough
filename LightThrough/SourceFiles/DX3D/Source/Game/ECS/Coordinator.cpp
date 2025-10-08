/**
 * @file ecs/Coordinator.cpp
 * @brief ECSを一元管理するクラス
 * @author Arima Keita
 * @date 2025-08-15
 */

 // ---------- インクルード ---------- // 
#include <Game/ECS/Coordinator.h>
#include <Game/ECS/Entity.h>
#include <Game/ECS/EntityManager.h>
#include <Game/ECS/ComponentManager.h>
#include <Game/ECS/SystemManager.h>

#include <Game/GameLogUtils.h>

namespace ecs {
	/**
	 * @brief 初期化
	 */
	void Coordinator::Init()
	{
		entity_manager_ = std::make_unique<EntityManager>();
		component_manager_ = std::make_unique<ComponentManager>();
		system_manager_ = std::make_unique<SystemManager>();
	}

	/**
	 * @brief Entityを生成
	 * @return 生成されたEntity
	 */
	Entity Coordinator::CreateEntity() {
		return entity_manager_->Create();
	}

	/**
	 * @brief Entityの破棄
	 * @param _e 対象のEntity
	 */
	void Coordinator::DestroyEntity(Entity _e)
	{
		if (!_e.IsInitialized() || !entity_manager_ || !entity_manager_->IsValid(_e)) {
			GameLogError("[ECS:Coordinator] Entityが未初期化 or すでに無効. Skip.");
			return;
		}
		// Entityが破棄されたことをComponentManagerとSystemManagerに通知
		// [ToDo] メソッド名が悪い
		component_manager_->EntityDestroyed(_e);
		system_manager_->EntityDestroyed(_e);
		entity_manager_->Destroy(_e);
	}

	/**
	 * @brief すべてのSystemを更新
	 * @param _dt デルタタイム
	 */
	void Coordinator::UpdateAllSystems(float _dt)
	{
		for (auto& [type, system] : system_manager_->GetAllSystems()) {
			system->Update(_dt);
		}
	}

	/**
	 * @brief 指定したSignatureを持っているEntityの一覧を取得
	 *	GetEntitiesWithComponentsから呼び出される形が多いと思われる
	 * @param _signature 指定するSignature
	 * @return EntityのVector型リスト
	 */
	std::vector<Entity> Coordinator::GetEntitiesWithSignature(Signature _signature)
	{
		std::vector<Entity> result;
		for (auto e : entity_manager_->GetAllEntities()) {
			if ((entity_manager_->GetSignature(e) & _signature) == _signature) {
				result.push_back(e);	// 持っているなら追加
			}
		}
		return result;
	}
}