#pragma once


// ---------- インクルード ---------- // 
#include <format>
#include <DX3D/Core/Logger.h>
#include <Game/Systems/MovementSystem.h>
#include <DX3D/Game/ECS/Coordinator.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Velocity.h>


ecs::MovementSystem::MovementSystem(dx3d::SystemDesc _desc)
	: ISystem(_desc)
{
	//// 必須コンポーネント
	//Signature signature;
	//signature.set(_ecs.GetComponentType<Transform>());
	//signature.set(_ecs.GetComponentType<Velocity>());
	//_ecs.SetSystemSignature<MovementSystem>(signature);
}

void ecs::MovementSystem::Update(float _dt, ecs::Coordinator& _ecs)
{
	for (auto const& e : entities_) {
		auto& t = _ecs.GetComponent<Transform>(e);
		auto& v = _ecs.GetComponent<Velocity>(e);
		float delta = v.velocity.x * _dt;
		t.position.x += delta;
		t.position.y += v.velocity.y * _dt;
		t.position.z += v.velocity.z * _dt;

	}
}
