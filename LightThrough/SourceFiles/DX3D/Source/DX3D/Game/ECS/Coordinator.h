#pragma once
/**
 * @file ecs/Coordinator.h
 * @brief ECSコーディネータ	
 * @author Arima Keita
 * @date 2025-08-15
 */

// ---------- インクルード ---------- //
#include <memory>
#include <DX3D/Game/ECS/EntityManager.h>
#include <DX3D/Game/ECS/ComponentManager.h>
#include <DX3D/Game/ECS/SystemManager.h>

namespace ecs {
	/**
	 * @brief ECSコーディネータ
	 *
	 * Entity、Component、Systemを管理する。
	 */
	class Coordinator final {
	public:
		void Init();		// 初期化
		Entity Create();	// Entityの生成

		template<typename Com>
		void RegisterComponent();	// Componentリストの登録
		template<typename Com>
		void AddComponent(Entity _e, const Com& _component);	// Componentの追加
		template<typename Com>
		void RegisterSystem();		// Systemの登録
		template<typename Com>
		void SetSystemSignature(Signature _signature);	// SystemのSignatureを設定
		template<typename Com>
		std::shared_ptr<Com> GetSystem();	// Systemの取得

	private:
		std::unique_ptr<EntityManager> entity_manager_{};		// Entityマネージャ
		std::unique_ptr<ComponentManager> component_manager_{};	// Componentマネージャ
		std::unique_ptr<SystemManager> system_manager_{};		// Systemマネージャ

	};


	/**
	 * @brief Componentリストの登録
	 * @<Com> Componentの種類
	 */
	template<typename Com>
	void Coordinator::RegisterComponent()
	{
		component_manager_->RegisterComponent<Com>();
	}

	/**
	 * @brief EntityにComponentを追加
	 * @<Com> 追加するComponentの種類
	 * @param _e 追加先のEntity
	 * @param _component 追加するComponentの参照
	 */
	template<typename Com>
	void AddComponent(Entity _e, const Com& _component)
	{
		component_manager_->AddComponent<Com>(_e, _component);

		// Signatureの更新
		auto sig = entity_manager_->GetSignature(_e);
		sig.set(component_manager_->GetComponentType<Com>(), true);
		entity_manager_->SetSignature(_e, sig);
		system_manager_->EntitySignatureChange(_e, sig);
	}


	/**
	 * @brief Systemの登録
	 * @<Com> 登録するSystemの種類
	 */
	template<typename Com>
	void RegisterSystem()
	{
		system_manager_->RegisterSystem<Com>();
	}

	/**
	 * @brief SystemのSignatureを設定
	 * @<Com> Signatureを設定するSystemの種類
	 * @param _signature 設定するSignature
	 */
	template<typename Com>
	void Coordinator::SetSystemSignature(Signature _signature)
	{
		system_manager_->SetSignature<Com>(_signature);
	}

	template<typename Com>
	std::shared_ptr<Com> Coordinator::GetSystem()
	{
		return system_manager_->GetSystem<Com>();
	}
}

