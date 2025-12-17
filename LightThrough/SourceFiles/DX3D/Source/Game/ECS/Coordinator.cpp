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
	 * @brief Entityの破棄リクエスト
	 * @param _e
	 */
	void Coordinator::RequestDestroyEntity(Entity _e)
	{
		pending_destroys_.push_back(_e);
	}

	/**
	 * @brief EntityにComponentを追加
	 * @param _e 追加先のEntity
	 * @param _type 追加するComponentの種類
	 * @param _data 追加するComponentのデータへのポインタ
	 */
	void Coordinator::AddComponent(Entity _e, ComponentType _type, const void* _data)
	{
		// 追加
		component_manager_->AddComponent(_e, _type, _data);
		
		// Signatureの更新
		auto sig = entity_manager_->GetSignature(_e);
		sig.set(_type, true);
		entity_manager_->SetSignature(_e, sig);
		system_manager_->EntitySignatureChanged(_e, sig);
	}
	
	/**
	 * @brief EntityからComponentを削除
	 * @param _e 削除先のEntity
	 * @param _type 削除するComponentの種類
	 */
	void Coordinator::RemoveComponent(Entity _e, ComponentType _type)
	{
		// 削除
		component_manager_->RemoveComponent(_e, _type);

		// Signatureの更新
		auto sig = entity_manager_->GetSignature(_e);
		sig.set(_type, false);
		entity_manager_->SetSignature(_e, sig);
		system_manager_->EntitySignatureChanged(_e, sig);
	}

	/**
	* @brief すべてのSystemを初期化
	*/
	void Coordinator::InitAllSystems()
	{
		for (auto& system : system_manager_->GetAllSystemsInOrder()) {
			system->Init();
		}
	}

	/**
	* @brief すべてのSystemを更新
	* @param _dt デルタタイム
	*/
	void Coordinator::FixedUpdateAllSystems(float _fixedDt)
	{
		for (auto& system : system_manager_->GetAllSystemsInOrder()) {
			system->FixedUpdate(_fixedDt);
		}
	}

	/**
	 * @brief すべてのSystemを更新
	 * @param _dt デルタタイム
	 */
	void Coordinator::UpdateAllSystems(float _dt)
	{
		for (auto& system : system_manager_->GetAllSystemsInOrder()) {
			system->Update(_dt);
		}
	}

	//! @brief 保留中の変更を反映
	void Coordinator::FlushPending()
	{
		// Component追加リクエストの反映
		for(auto& add: pending_adds_) {
			add.apply();
		}
		pending_adds_.clear();
		// Component削除リクエストの反映
		for (auto& remove : pending_removes_) {
			RemoveComponent(remove.e, remove.type);
		}
		pending_removes_.clear();
		// Entity破棄リクエストの反映
		for(auto& e: pending_destroys_) {
			DestroyEntity(e);
		}
		pending_destroys_.clear();
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
		for (auto& e : entity_manager_->GetAllEntities()) {
			if ((entity_manager_->GetSignature(e) & _signature) == _signature) {
				result.push_back(e);	// 持っているなら追加
			}
		}
		return result;
	}
}