#pragma once

#include <DirectXMath.h>
#include <Game/Serialization/ComponentReflection.h>


namespace ecs {
	struct EnvironmentLight {
		DirectX::XMFLOAT3 ambientColor{ 0.1f, 0.1f, 0.1f };
		float _pad0 = 0.0f;
	};
}


ECS_REFLECT_BEGIN(ecs::EnvironmentLight)
ECS_REFLECT_FIELD(ambientColor)
ECS_REFLECT_END()