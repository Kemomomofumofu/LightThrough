/**
 * @file PrefabSystem.h
 * @brief オブジェクトを生成したり
 * @author Arima Keita
 * @date 2025-08-20
 */


 // ---------- インクルード ---------- // 
#include <Game/ECS/Coordinator.h>
#include <Game/ECS/Entity.h>

#include <Game/Factorys/PrefabFactory.h>

#include <Game/Components/Core/Transform.h>
#include <Game/Components/Camera/Camera.h>
#include <Game/Components/Input/CameraController.h>
#include <Game/Components/Render/Light.h>

namespace ecs {
	Entity PrefabFactory::CreatePlacedLight(const PlacedLightDesc& _desc)
	{
		Entity e = ecs_.CreateEntity();

		Transform tf{};
		tf.SetPosition(_desc.pos);
		tf.SetRotationFromDirection(_desc.dir);
		ecs_.RequestAddComponent(e, tf);

		LightCommon light{};
		ecs_.RequestAddComponent(e, light);

		SpotLight spot{};
		ecs_.RequestAddComponent(e, spot);

		return e;
	}
}