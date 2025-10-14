#pragma once
/**
* @file Coordinator.inl
* @brief ECSコーディネータのテンプレート関数の定義
* @author Arima Keita
* @date 2025-08-16
*/


// ---------- インクルード ---------- // 
#include <Game/ECS/Coordinator.h>
#include <Game/ECS/ECSUtils.h>
#include <Game/ECS/EntityManager.h>
#include <Game/ECS/ComponentManager.h>
#include <Game/ECS/SystemManager.h>

namespace ecs {
	/**
	 * @brief Componentリストの登録
	 * @param <Com> Componentの種類
	 */
	template<typename Com>
	void Coordinator::RegisterComponent()
	{
		component_manager_->RegisterComponent<Com>();
	}

	/**
	 * @brief EntityにComponentを追加
	 * @param <Com> 追加するComponentの種類
	 * @param _e 追加先のEntity
	 * @param _component 追加するComponentの参照
	 */
	template<typename Com>
	void Coordinator::AddComponent(Entity _e, const Com& _component)
	{
		component_manager_->AddComponent<Com>(_e, _component);

		// Signatureの更新
		auto sig = entity_manager_->GetSignature(_e);
		sig.set(component_manager_->GetComponentType<Com>(), true);
		entity_manager_->SetSignature(_e, sig);
		system_manager_->EntitySignatureChanged(_e, sig);
	}

	/**
	 * @brief EntityからComponentを取得
	 * @param <Com> 取得するComponentの種類
	 * @param _e 削除先のEntity
	 */
	template<typename Com>
	Com& Coordinator::GetComponent(Entity _e)
	{
		return component_manager_->GetComponent<Com>(_e);
	}


	/**
	 * @brief 指定されたComponentを持っているEntityを取得
	 * @param <Com> 指定するComponent
	 * @return EntityのVector型リスト
	 */
	template<typename Com>
	inline std::vector<Entity> Coordinator::GetEntitiesWithComponent()
	{
		std::vector<Entity> result;
		for (const auto& e : entity_manager_->GetAllEntities()) {
			if (component_manager_->HasComponent<Com>(e)) {
				result.push_back(e);	// 持っているなら追加
			}
		}

		return result;
	}


	/**
	 * @brief 指定された複数のComponentを持っているEntityを取得
	 *	Signatureに変換して、GetEntitiesWithSignature()を呼び出す形
	 * @param <...Coms> 指定する複数のComponent
	 * @return EntityのVector型リスト
	 */
	template<typename ...Coms>
	inline std::vector<Entity> Coordinator::GetEntitiesWithComponents()
	{
		Signature sig;
		(sig.set(component_manager_->GetComponentType<Coms>()), ...);
		return GetEntitiesWithSignature(sig);
	}

	/**
	 * @brief EntityがComponentを持っているか確認
	 * @param <Com> 確認するComponentの種類
	 * @param _e 確認するEntity
	 * @return true: 持っている, false: 持っていない
	 * 
	 * [ToDo] 直接ComponentArrayの中身を探しているので、Bitsetで管理するようにして高速化を図るべき
	 */
	template<typename Com>
	bool Coordinator::HasComponent(Entity _e)
	{
		return component_manager_->HasComponent<Com>(_e);
	}

	/**
	 * @brief ComponentのTypeを取得
	 * @param <Com> 取得するComponentの種類
	 * @return ComponentのType
	 */
	template<typename Com>
	ComponentType Coordinator::GetComponentType()
	{
		return component_manager_->GetComponentType<Com>();
	}

	/**
	 * @brief Systemの登録
	 * @param <Sys> 登録するSystemの種類
	 */
	template<typename Sys>
	void Coordinator::RegisterSystem(const SystemDesc& _desc)
	{
		system_manager_->RegisterSystem<Sys>(_desc);
	}

	/**
	 * @brief SystemのSignatureを設定
	 * @param <Sys> Signatureを設定するSystemの種類
	 * @param _signature 設定するSignature
	 */
	template<typename Sys>
	void Coordinator::SetSystemSignature(Signature& _signature)
	{
		system_manager_->SetSignature<Sys>(_signature);
	}

	/**
	 * @brief Systemを取得
	 * @param <Sys> 取得するSystemの種類
	 * @return 取得したSystemのポインタ
	 */
	template<typename Sys>
	std::shared_ptr<Sys> Coordinator::GetSystem()
	{
		return system_manager_->GetSystem<Sys>();
	}
}