/**
 * @file ecs/Coordinator.cpp
 * @brief ECSを一元管理するクラス
 * @author Arima Keita
 * @date 2025-08-15
 */

// ---------- インクルード ---------- // 
#include <DX3D/Game/ECS/Coordinator.h>



/**
 * @brief 初期化
 */
void ecs::Coordinator::Init()
{
	entity_manager_ = std::make_unique<EntityManager>();
	component_manager_ = std::make_unique<ComponentManager>();
	system_manager_ = std::make_unique<SystemManager>();
}

/**
 * @brief Entityを生成
 * @return 生成されたEntity
 */
ecs::Entity ecs::Coordinator::CreateEntity() {
	return entity_manager_->Create();
}

/**
 * @brief Entityの破棄
 * @param _e 対象のEntity
 */
void ecs::Coordinator::DestroyEntity(Entity _e)
{
	entity_manager_->Destroy(_e);
	component_manager_->EntityDestroyed(_e);
	system_manager_->EntityDestroyed(_e);
}
