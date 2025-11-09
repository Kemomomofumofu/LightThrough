/**
 * @file PlayerControllerSystem.h
 * @brief プレイヤーを操作するシステム
 */

// ---------- インクルード ---------- // 
#include <Game/Systems/PlayerControllerSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/InputSystem/InputSystem.h>

#include <Game/Components/PlayerController.h>
#include <Game/Components/Transform.h>

namespace ecs {
	void PlayerControllerSystem::Init()
	{
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<PlayerController>());
		ecs_.SetSystemSignature<PlayerControllerSystem>(signature);
	}

	// todo: また実装する
	void PlayerControllerSystem::Update(float _dt)
	{
		using namespace DirectX;
		auto& input = input::InputSystem::Get();


		
	}
}