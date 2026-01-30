#pragma once
/**
 * @file PrefabFactory.h
 * @brief オブジェクトを生成する担当
 */

 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>

#include <Game/ECS/Coordinator.h>

namespace ecs {
	struct PlacedLightDesc
	{
		DirectX::XMFLOAT3 pos{};
		DirectX::XMFLOAT3 dir{};
	};

	class PrefabFactory final {
	public:
		explicit PrefabFactory(ecs::Coordinator& _coordinator)
			: ecs_(_coordinator) { }

		Entity CreatePlacedLight(const PlacedLightDesc& _desc);

	private:
		ecs::Coordinator& ecs_;
	};


}