#pragma once
/**
 * @file ecs/Coordinator.h
 * @brief ECSコーディネータ	
 * @author Arima Keita
 * @date 2025-08-15
 */

// ---------- インクルード ---------- //
#include <memory>
#include <DX3D/Game/ECS/ECSUtils.h>

namespace ecs {
		
	// ---------- 前方宣言 ---------- // 
	class Entity;
	class EntityManager;
	class ComponentManager;
	class SystemManager;

	/**
	 * @brief ECSコーディネータ
	 *
	 * Entity、Component、Systemを管理する。
	 */
	class Coordinator final {
	public:
		void Init();		// 初期化

		Entity CreateEntity();	// Entityの生成
		void DestroyEntity(Entity _e);	// Entityの破棄

		template<typename Com>
		void RegisterComponent();	// Componentリストの登録
		template<typename Com>
		void AddComponent(Entity _e, const Com& _component);	// Componentの追加
		template<typename Com>
		Com& GetComponent(Entity _e);	// Componentの取得
		template<typename Com>
		bool HasComponent(Entity _e);	// Componentを持っているか
		template<typename Com>
		ComponentType GetComponentType();	// ComponentのTypeを取得

		template<typename Sys>
		void RegisterSystem();		// Systemの登録
		template<typename Sys>
		void SetSystemSignature(Signature _signature);	// SystemのSignatureを設定
		template<typename Sys>
		std::shared_ptr<Sys> GetSystem();	// Systemの取得


	private:
		std::unique_ptr<EntityManager> entity_manager_{};		// Entityマネージャ
		std::unique_ptr<ComponentManager> component_manager_{};	// Componentマネージャ
		std::unique_ptr<SystemManager> system_manager_{};		// Systemマネージャ

	};
}

#include <DX3D/Game/ECS/Coordinator.inl>

