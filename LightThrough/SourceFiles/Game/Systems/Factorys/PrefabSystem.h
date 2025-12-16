#pragma once
/**
 * @file PrefabSystem
 * @brief オブジェクトを生成する担当
 * @author Arima Keita
 * @date 2025-08-20
 */

 // ---------- インクルード ---------- // 
#include <Game/GameObjectType.h>
#include <Game/ECS/ISystem.h>


namespace ecs {
// ---------- 前方宣言 ---------- //
	class Coordinator;
	struct Entity;

	class PrefabSystem final : public ecs::ISystem{
	public:
		PrefabSystem(const ecs::SystemDesc& _desc)
			: ISystem(_desc){ }
		void Update(float _dt) override;

		ecs::Entity CreateGameObject(LightThrough::GameObjectType _type);	// ゲームオブジェクトを生成する

	private:
		ecs::Entity CreatePlayer();
		ecs::Entity CreateCamera();
	};


}