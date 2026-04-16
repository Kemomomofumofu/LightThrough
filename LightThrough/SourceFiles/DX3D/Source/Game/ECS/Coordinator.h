#pragma once
/**
 * @file ecs/Coordinator.h
 * @brief ECSコーディネータ
 * @author Arima Keita
 * @date 2025-08-15
 */

 // ---------- インクルード ---------- //
#include <vector>
#include <memory>
#include <functional>
#include <DX3D/Core/Common.h>
#include <Game/ECS/ECSUtils.h>
#include <Game/ECS/ISystem.h>

namespace ecs {
	// ---------- 前方宣言 ---------- // 
	struct Entity;
	class EntityManager;
	class ComponentManager;
	class SystemManager;

	/**
	 * @brief ECSコーディネータ
	 *
	 * Entity、Component、Systemを管理する。
	 */
	class Coordinator final : public dx3d::Base {
	public:
		Coordinator(const dx3d::BaseDesc& _desc) : Base(_desc) {}

		void Init();

		// ---------- Entity関連 ---------- //
		/**
		 * @brief Entityの生成
		 * @return 生成されたEntity
		 */
		Entity CreateEntity();
		/**
		 * @brief Entityの破棄
		 * @param _e: 破棄するEntity
		 */
		void DestroyEntity(Entity _e);
		/**
		 * @brief 登録されている全てのEntityを取得
		 * @return 登録されている全てのEntityのコンテナ
		 */
		std::vector<Entity> GetAllEntities();
		/**
		 * @brief Entityが有効かどうかを確認
		 * @param _e: 確認するEntity
		 * @return true: 有効, false: 無効
		 */
		template<typename Com>
		const std::vector<Entity>& GetEntitiesWithComponent();
		/**
		 * @brief 複数のコンポーネントを持っているEntityの一覧を取得
		 * @tparam ...Coms : コンポーネントの型リスト
		 * @return 該当するEntityの一覧
		 */
		template<typename... Coms>
		std::vector<Entity> GetEntitiesWithComponents();
		/**
		 * @brief Entityの破棄リクエスト
		 * @param _e: 破棄するEntity
		 */
		void RequestDestroyEntity(Entity _e);

		/**
		 * @brief 指定したSignatureを持っているEntityの一覧を取得
		 * @param _signature: 指定するSignature
		 * @return 該当するEntityの一覧
		 */
		std::vector<Entity> GetEntitiesWithSignature(Signature _signature);

		/**
		 * @brief EntityがValidかどうかを確認
		 * @param _e : 確認するEntity
		 * @return true: 有効, false: 無効
		 */
		bool IsValidEntity(Entity _e);

		// ---------- Component関連 ---------- //
		template<typename Com>
		void RegisterComponent();	// Componentリストの登録

		template<typename Com>
		Com* AddComponent(Entity _e, const Com& _component);	// Componentの追加
		void AddComponentRaw(Entity _e, ComponentType _type, const void* _data);

		template<typename Com>
		void RemoveComponent(Entity _e);	// Componentの削除
		void RemoveComponent(Entity _e, ComponentType _type);
		/**
		 * @brief コンポーネントを持っているか
		 * @param _確認先Entity
		 * @return true: 持っている, false: 持っていない
		 */
		template<typename Com>
		bool HasComponent(Entity _e);

		template<typename Com>
		Com* GetComponent(Entity _e);	// Componentの取得

		template<typename Com>
		ComponentType GetComponentType();	// ComponentのTypeを取得

		/**
		 * @brief 指定した型のComponentを全走査してコールバックを呼び出す
		 * @tparam Com コンポーネントの種類
		 * @param _func コールバック関数 (Entity, const Com&) を受け取る
		 */
		template<typename Com>
		void ForEachComponent(const std::function<void(Entity, const Com&)>& _func);

		/**
		 * @brief コンポーネントの追加リクエスト
		 * @param _e 対象のEntity
		 * @param _type 追加するComponentのType
		 * @param _apply 追加処理
		 */
		void RequestAddComponentRaw(Entity _e, ComponentType _type, std::function<void()> _apply);
		template<typename Com>
		void RequestAddComponent(Entity _e, const Com& _component);
		template<typename Com>
		void RequestRemoveComponent(Entity _e);	// Componentの削除リクエスト


		// ---------- System関連 ---------- //
		template<typename Sys>
		void RegisterSystem(const SystemDesc& _desc);		// Systemの登録
		template<typename Sys>
		void SetSystemSignature(Signature& _signature);	// SystemのSignatureを設定
		template<typename Sys>
		std::shared_ptr<Sys> GetSystem();	// Systemの取得
		void ReactivateAllSystems(); // 登録されたSystemをすべて再アクティブ化

		const std::vector<std::shared_ptr<ISystem>>& GetAllSystems();



		void InitAllSystems();	// 登録されたSystemの初期化
		void FixedUpdateAllSystems(float _fixedDt); // 登録されたSystemの固定更新
		void UpdateAllSystems(float _dt);	// 登録されたSystemの更新
		void FlushPending(); // 保留中の変更を反映



		// ---------- コールバック設定 ---------- //
		using OnEntityDestroyed = std::function<void(Entity)>;
		void SetOnEntityDestroyedCallback(OnEntityDestroyed _callback) {on_entity_destroyed_ = std::move(_callback); }

	private:
		/**
		 * @brief 保留中の追加処理
		 */
		struct PendingAdd {
			Entity e;
			ComponentType type;
			std::function<void()> apply;
		};

		/**
		 * @brief 保留中の削除処理
		 */
		struct PendingRemove {
			Entity e;
			ComponentType type;
		};

		std::unique_ptr<EntityManager> entity_manager_{};		// Entityマネージャ
		std::unique_ptr<ComponentManager> component_manager_{};	// Componentマネージャ
		std::unique_ptr<SystemManager> system_manager_{};		// Systemマネージャ

		std::vector<PendingAdd> pending_adds_{};		// 保留中の追加処理
		std::vector<PendingRemove> pending_removes_{};	// 保留中の削除処理
		std::vector<Entity> pending_destroys_{};		// 保留中の破棄処理

		OnEntityDestroyed on_entity_destroyed_{};	// Entity破棄時のコールバック

	};
}

#include <Game/ECS/Coordinator.inl>

