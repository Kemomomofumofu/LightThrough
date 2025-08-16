/**
 * @file EntityManager.cpp
 * @brief エンティティマネージャ
 * @author Arima Keita
 * @date 2025-08-06
 */

// ---------- インクルード ---------- // 
#include <cassert>
#include <DX3D/Game/ECS/EntityManager.h>


/**
 * @brief エンティティの生成
 * @return エンティティ
 */
ecs::Entity ecs::EntityManager::Create()
{
	uint32_t index;

	// 再利用可能なIndexがあるなら
	if (!free_index_.empty()) {
		index = free_index_.front();
		free_index_.pop();
	}
	// ないなら
	else {
		index = ++next_index_;	// 新規のIndex
		versions_.push_back(0);	// 初期Version
	}

	uint32_t version = versions_[index];
	// Entityを生成
	return Entity{ ecs::CreateEntity(index, version) };
}


/**
 * @brief エンティティの破棄
 * @param _entity 破棄するエンティティ
 */
void ecs::EntityManager::Destroy(Entity _entity)
{
	uint32_t index = _entity.Index();	// EntityのIndexを取得
	if (index >= versions_.size()) { return; }

	++versions_[index];	// Versionを進めて無効化
	free_index_.push(index);	// Indexを再利用可能リストへ
}

/**
 * @brief EntityのSignatureを設定
 * @param _entity 対象のEntity
 * @param _signature 設定するSignature
 */
void ecs::EntityManager::SetSignature(Entity _entity, Signature _signature)
{
	assert(_entity.Index() < signatures_.size());
	signatures_[_entity.Index()] = _signature;	// Signatureを設定
}

/**
 * @brief EntityのSignatureを取得
 * @param _entity 対象のEntity
 * @return Signature
 */
ecs::Signature ecs::EntityManager::GetSignature(Entity _entity) const
{
	assert(_entity.Index() < signatures_.size());
	return signatures_[_entity.Index()];	// Signatureを返す
}


/**
 * @brief Entityが有効かどうか
 * @param _entity 確認するEntity
 * @return 有効: true, 無効: false
 */
bool ecs::EntityManager::IsValid(Entity _entity) const
{
	uint32_t index = _entity.Index();	// EntityのIndexを取得
	if (index >= versions_.size()) { return false; }

	// Versionが同じなら有効
	return versions_[index] == _entity.Version();
}


