#pragma once

#include <DirectXMath.h>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {

	struct DirectionalLight {
		DirectX::XMFLOAT3 directionWS{ 0.5f, -1.0f, 0.2f };
		float _pad0 = 0.0f;
		DirectX::XMFLOAT3 color{ 1.0f, 1.0f, 1.0f };
		float _pad1 = 0.0f;
	};

}