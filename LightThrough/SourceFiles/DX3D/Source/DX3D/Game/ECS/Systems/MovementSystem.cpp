#pragma once


// ---------- インクルード ---------- // 
#include <format>
#include <DX3D/Core/Logger.h>
#include <DX3D/Game/ECS/Systems/MovementSystem.h>
#include <DX3D/Game/ECS/Coordinator.h>
#include <DX3D/Game/ECS/Components/Transform.h>
#include <DX3D/Game/ECS/Components/Velocity.h>


void ecs::MovementSystem::Update(float _dt, ecs::Coordinator& _coord)
{
	for (auto const& e : entities_) {
		auto& t = _coord.GetComponent<Transform>(e);
		auto& v = _coord.GetComponent<Velocity>(e);
		float delta = v.velocity.x * _dt;
		t.position.x += delta;
		t.position.y += v.velocity.y * _dt;
		t.position.z += v.velocity.z * _dt;

	}
}
