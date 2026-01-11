#pragma once


// ---------- ÉCÉìÉNÉãÅ[Éh ---------- // 
#include <DirectXMath.h>
#include <Game/Serialization/ComponentReflection.h>


namespace ecs {
	struct Rigidbody {
		DirectX::XMFLOAT3 linearVelocity{};
		DirectX::XMFLOAT3 angularVelocity{};
		DirectX::XMFLOAT3 force{};
		DirectX::XMFLOAT3 torque{};

		float mass = 1.0;			// éøó 
		float drag = 0.0f;			// íÔçR
		float angularDrag = 0.0f;	// íÔçR
		float restitution = 0.0f;	// îΩî≠åWêî
		float friction = 0.0f;		// ñÄéC

		bool useGravity = true;
		bool isStatic = false;
		bool isKinematic = false;
	};
}


ECS_REFLECT_BEGIN(ecs::Rigidbody)
ECS_REFLECT_FIELD(mass),
ECS_REFLECT_FIELD(drag),
ECS_REFLECT_FIELD(angularDrag),
ECS_REFLECT_FIELD(restitution),
ECS_REFLECT_FIELD(friction),
ECS_REFLECT_FIELD(useGravity),
ECS_REFLECT_FIELD(isStatic),
ECS_REFLECT_FIELD(isKinematic)
ECS_REFLECT_END()


