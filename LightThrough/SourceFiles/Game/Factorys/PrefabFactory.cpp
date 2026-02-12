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
#include <Game/Components/Render/MeshRenderer.h>

namespace ecs {
	Entity PrefabFactory::CreatePlacedLight(const PlacedLightDesc& _desc)
	{
		Entity e = ecs_.CreateEntity();

		Transform tf{};
		tf.SetPosition(_desc.pos);
		tf.SetRotationFromDirection(_desc.dir);
		tf.SetScale({ 0.25f, 0.25f, 0.25f });
		ecs_.RequestAddComponent(e, tf);

		LightCommon light{};
		ecs_.RequestAddComponent(e, light);

		SpotLight spot{};
		ecs_.RequestAddComponent(e, spot);

		// todo: 将来的には影を落とさないものとして、ライトの位置を描画したい。
		//MeshRenderer mr{};
		//mr.meshName = "Sphere";
		//ecs_.RequestAddComponent(e, mr);


		return e;
	}
}