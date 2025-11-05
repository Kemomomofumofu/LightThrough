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
#include <Game/Components/MeshRenderer.h>
#include <DX3D/Graphics/Meshes/PrimitiveFactory.h>


#include <Debug/DebugUI.h>

namespace ecs {
	/**
	 * @brief コンストラクタ
	 * @param _desc
	 */
	TitleSceneSystem::TitleSceneSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{


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
	}
}
