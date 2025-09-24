/**
 * @file TitleSceneSystem.cpp
 * @brief タイトルシーンのふるまい
 * @author Arima Keita
 * @date 2025-09-22
 */

 // ---------- インクルード ---------- // 
#include <Game/Systems/Scenes/TitleSceneSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Transform.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>
#include <Game/Components/Mesh.h>
#include <DX3D/Graphics/PrimitiveFactory.h>


#include <Debug/DebugUI.h>

namespace ecs {
	/**
	 * @brief コンストラクタ
	 * @param _desc
	 */
	TitleSceneSystem::TitleSceneSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
//		// Entityの生成
//// カメラ
//		auto eCamera = ecs_.CreateEntity();
//		ecs_.AddComponent<ecs::Transform>(eCamera, ecs::Transform{ {0.0f, 0.0f, -5.0f}, {0.0f, 0.0f, 0.0f} });
//		ecs_.AddComponent<ecs::Camera>(eCamera, ecs::Camera{});
//		ecs_.AddComponent<ecs::CameraController>(eCamera, ecs::CameraController{ ecs::CameraMode::FPS });
//		scene_manager_->AddEntityToScene(sceneId, eCamera);
//
//		// テストのメッシュ
//		auto e = ecs_.CreateEntity();
//		ecs_.AddComponent<ecs::Transform>(e, ecs::Transform{ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} });
//		auto mesh = dx3d::PrimitiveFactory::CreateSphere(graphics_engine_->GetGraphicsDevice(), 32, 16);
//		ecs_.AddComponent<ecs::Mesh>(e, mesh);

	}

	/**
	 * @brief 更新
	 * 
	 * ここにタイトルシーンのふるまいを実装する
	 * 
	 * @param _dt デルタタイム
	 */
	void TitleSceneSystem::Update(float _dt)
	{
	}

	void TitleSceneSystem::DebugTitleScene()
	{
		ImGui::Begin("Title Scene Debug");
		ImGui::Text("Title Scene System");
		ImGui::End();
	}
}
