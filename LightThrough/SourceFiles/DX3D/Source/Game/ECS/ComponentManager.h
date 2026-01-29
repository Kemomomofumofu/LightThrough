#pragma once
/**
 * @file ComponentManager.h
 * @brief コンポーネントマネージャ
 * @author Arima Keita
 * @date 2025-08-06
 */

 // ---------- インクルード ---------- //
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <cassert>
#include <array>
#include <Game/ECS/ECSUtils.h>

namespace ecs {
	// ---------- 前方宣言 ---------- //
	struct Entity;			// Entityクラス
	class IComponentArray;	// コンポーネント配列のインターフェース
	template<typename Com> class ComponentArray;	// コンポーネント配列クラス

	/**
	 * @brief コンポーネントマネージャ
	 *
	 * コンポーネントの管理を担当するクラス
	 * 各コンポーネントのリストを保持し、Entityに対してコンポーネントの追加、削除、取得を行う。
	 */
	class ComponentManager final{
	public:
		template<typename Com>
		void RegisterComponent();	// Componentリストの登録
		template<typename Com>
		Com* GetComponent(Entity _e);	// Componentの取得
		template<typename Com>
		bool HasComponent(Entity _e);	// Componentを持っているか
		template<typename Com>
		ComponentType GetComponentType();	// ComponentのTypeを取得

		void AddComponent(Entity _e, ComponentType _type, const void* _data);
		void RemoveComponent(Entity _e, ComponentType _type);	// Componentの削除
		void EntityDestroyed(Entity _e);	// Entityが破棄された際に呼び出す

	private:
		template<typename Com>
		ComponentArray<Com>* GetComponentArray();	// Componentリストの取得

	private:
		std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> component_arrays_;	// Componentリストを保持するMap
		std::unordered_map<std::type_index, ComponentType> component_types_;	// コンポーネントに対応する整数を保持するMap
		std::array<IComponentArray*, MAX_COMPONENTS> component_arrays_by_type_{}; // ComponentTypeからComponentArrayを取得するための配列
		ComponentType next_component_type_ = 0;	// 次に登録されるコンポーネントのType
	};

}

#include <Game/ECS/ComponentManager.inl>
