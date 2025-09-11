#pragma once
/**
 * @file SystemManager.h
 * @brief システム達を管理するクラス
 * @author Arima Keita
 * @date 2025-08-08
 */

// ---------- インクルード ---------- // 
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <cassert>
#include <DX3D/Game/ECS/ECSUtils.h>

namespace ecs {
	// ---------- 前方宣言 ---------- //
	class Coordinator;
	class ISystem;
	struct Entity;

	/**
	 * @brief システムマネージャ
	 *
	 * システムの登録、管理やシステムに必要なコンポーネントの管理をする。
	 */
	class SystemManager final{
	public:
		template<typename T>
		std::shared_ptr<T> RegisterSystem(ecs::Coordinator& _ecs);
		template<typename T> 
		void SetSignature(Signature _signature);	// システムのSignatureを設定
		template<typename T>
		std::shared_ptr<T> GetSystem();		// システムを取得
		const std::unordered_map<std::type_index, std::shared_ptr<ISystem>>& GetAllSystems() const;

		void EntitySignatureChanged(Entity _e, Signature _eSignature);	// EntityのSignatureが変わった時に呼び出す
		void EntityDestroyed(Entity _e);	// Entityが破棄された時に呼び出す


	private:
		std::unordered_map<std::type_index, Signature> signature_;
		std::unordered_map<std::type_index, std::shared_ptr<ISystem>> systems_;
	};
}

#include <DX3D/Game/ECS/SystemManager.inl>