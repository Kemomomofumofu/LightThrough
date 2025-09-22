#pragma once


// ---------- インクルード ---------- // 
#include <format>
#include <DX3D/Core/Logger.h>
#include <Game/Systems/MovementSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Velocity.h>


ecs::MovementSystem::MovementSystem(SystemDesc _desc)
	: ISystem(_desc)
{
	//// 必須コンポーネント
	//Signature signature;
	//signature.set(_ecs.GetComponentType<Transform>());
	//signature.set(_ecs.GetComponentType<Velocity>());
	//_ecs.SetSystemSignature<MovementSystem>(signature);
}

void ecs::MovementSystem::Update(float _dt)
{
	for (auto const& e : entities_) {
		auto& t = ecs_.GetComponent<Transform>(e);
		auto& v = ecs_.GetComponent<Velocity>(e);
		float delta = v.velocity.x * _dt;
		t.position.x += delta;
		t.position.y += v.velocity.y * _dt;
		t.position.z += v.velocity.z * _dt;

	}
}
