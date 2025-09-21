/**
 * @file EntityManager.cpp
 * @brief エンティティマネージャ
 * @author Arima Keita
 * @date 2025-08-06
 */

 // ---------- インクルード ---------- // 
#include <Game/ECS/EntityManager.h>
#include <cassert>
#include <stdexcept>
#include <Game/ECS/ECSUtils.h>


namespace ecs {
	EntityManager::EntityManager()
	{
		versions_.resize(1);	// 初期化: 0番目のIndexはNull扱い
		versions_[0] = 0;

		signatures_.resize(1);
	}

	/**
	 * @brief エンティティの生成
	 * @return エンティティ
	 */
	Entity EntityManager::Create()
	{
		uint32_t index = 0;

		// 再利用可能なIndexがあるなら
		if (!free_index_.empty()) {
			index = free_index_.front();
			free_index_.pop();
		}
		// ないなら
		else {
			// indexは1から始まる
			index = ++next_index_;	// 新規のIndex

			const uint32_t maxIndex = (1u << ecs::INDEX_BITS) - 1u;	// 最大Index値
			if (index > maxIndex) {
				throw std::runtime_error("エンティティの最大数を超えました");
			}

			EnsureCapacityForIndex(index);	// 必要な容量を確保
			versions_[index] = 0;	// 新規IndexのVersionを初期化
			signatures_[index].reset();	// Signatureを初期化
		}

		uint32_t version = versions_[index];
		// Entityを生成
		return Entity{ ecs::CreateEntity(index, version) };
	}


	/**
	 * @brief エンティティの破棄
	 * @param _entity 破棄するエンティティ
	 */
	void EntityManager::Destroy(Entity _entity)
	{
		uint32_t index = _entity.Index();	// EntityのIndexを取得
		if (index == 0) { return; }	// Indexが0ならnull扱いなので何もしない
		if (index >= versions_.size()) { return; }

		versions_[index] = (versions_[index] + 1u) & ecs::VERSION_MASK;	// Versionを進めて無効化
		if (index < signatures_.size()) {
			signatures_[index].reset();	// Signatureを初期化
		}
		free_index_.push(index);	// Indexを再利用可能リストへ
	}

	/**
	 * @brief Entityが有効かどうか
	 * @param _entity 確認するEntity
	 * @return 有効: true, 無効: false
	 */
	bool EntityManager::IsValid(Entity _entity) const
	{
		uint32_t index = _entity.Index();	// EntityのIndexを取得
		if (index == 0) { return false; }
		if (index >= versions_.size()) { return false; }

		// Versionが同じなら有効
		return versions_[index] == _entity.Version();
	}


	/**
	 * @brief 有効なEntityのリストを取得
	 * @return 
	 */
	std::vector<Entity> EntityManager::GetAllEntities()
	{
		std::vector<Entity> result;

		for (uint32_t i = 1; i < static_cast<uint32_t>(versions_.size()); ++i) {
			Entity e = Entity(CreateEntity(i, versions_[i]));		// VersionとIndexを合わせてEntityを再構築
			if (IsValid(e)) {
				result.push_back(e);	// 有効なもののみリストに追加
			}
		}

		return result;
	}

	/**
	 * @brief EntityのSignatureを設定
	 * @param _entity 対象のEntity
	 * @param _signature 設定するSignature
	 */
	void EntityManager::SetSignature(Entity _entity, const Signature& _signature)
	{
		uint32_t index = _entity.Index();
		if (index == 0) { return; }
		EnsureCapacityForIndex(index);	// 必要な容量を確保
		signatures_[index] = _signature;	// Signatureを設定
	}

	/**
	 * @brief EntityのSignatureを取得
	 * @param _entity 対象のEntity
	 * @return Signature
	 */
	Signature EntityManager::GetSignature(Entity _entity) const
	{
		uint32_t index = _entity.Index();
		if (index == 0) { return {}; }
		return signatures_[index];	// Signatureを返す
	}

	/**
	 * @brief 指定したIndexのために必要な容量を確保する
	 * @param _index 確保するIndex
	 */
	void EntityManager::EnsureCapacityForIndex(uint32_t _index)
	{
		if (_index >= versions_.size()) {
			// サイズを拡張
			versions_.resize(_index + 1);
			signatures_.resize(_index + 1);
		}
	}

}