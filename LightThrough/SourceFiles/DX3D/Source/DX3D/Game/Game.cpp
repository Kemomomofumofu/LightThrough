/**
 * @file Game.cpp
 * @brief ゲーム
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Game/Game.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Game/Display.h>
#include <DX3D/Math/Point.h>
#include <Game/Scene/SceneManager.h>
#include <Game/InputSystem/InputSystem.h>
#include <Game/Serialization/ComponentReflection.h>

#include <Game/Systems/TransformSystem.h>
#include <Game/Systems/RenderSystem.h>
#include <Game/Systems/CameraSystem.h>
#include <Game/Systems/DebugRenderSystem.h>
#include <Game/Systems/Collisions/ColliderSyncSystem.h>
#include <Game/Systems/Collisions/CollisionResolveSystem.h>
#include <Game/Systems/Physics/ForceAccumulationSystem.h>
#include <Game/Systems/Physics/IntegrationSystem.h>
#include <Game/Systems/Physics/ClearForcesSystem.h>

#include <Game/Components/MeshRenderer.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>
#include <Game/Components/Collider.h>
#include <Game/Components/Physics/Rigidbody.h>

#include <Game/GameLogUtils.h>
#include <Debug/DebugUI.h>
#include <Debug/Debug.h>

#pragma region ローカルメソッド
namespace {
	/**
	 * @brief システムの登録
	 * @param _ecs ECSのコーディネーター
	 */
	void RegisterAllSystems(ecs::SystemDesc& _desc, dx3d::GraphicsEngine& _engine)
	{
		auto& ecsCoordinator = _desc.ecs;


		// ---------- 衝突関係 ---------- // 
		ecsCoordinator.RegisterSystem<ecs::ForceAccumulationSystem>(_desc);
		const auto& forceAccumulationSystem = ecsCoordinator.GetSystem<ecs::ForceAccumulationSystem>();
		forceAccumulationSystem->Init();

		ecsCoordinator.RegisterSystem<ecs::IntegrationSystem>(_desc);
		const auto& integrationSystem = ecsCoordinator.GetSystem<ecs::IntegrationSystem>();
		integrationSystem->Init();

		ecsCoordinator.RegisterSystem<ecs::ColliderSyncSystem>(_desc);
		const auto& colliderSyncSystem = ecsCoordinator.GetSystem<ecs::ColliderSyncSystem>();
		colliderSyncSystem->Init();

		ecsCoordinator.RegisterSystem<ecs::CollisionResolveSystem>(_desc);
		const auto& collisionResolveSystem = ecsCoordinator.GetSystem<ecs::CollisionResolveSystem>();
		collisionResolveSystem->Init();

		ecsCoordinator.RegisterSystem<ecs::ClearForcesSystem>(_desc);
		const auto& clearForcesSystem = ecsCoordinator.GetSystem<ecs::ClearForcesSystem>();
		clearForcesSystem->Init();


		// カメラ
		ecsCoordinator.RegisterSystem<ecs::CameraSystem>(_desc);
		const auto& cameraSystem = ecsCoordinator.GetSystem<ecs::CameraSystem>();
		cameraSystem->Init();

		// 位置更新
		ecsCoordinator.RegisterSystem<ecs::TransformSystem>(_desc);
		const auto& transformSystem = ecsCoordinator.GetSystem<ecs::TransformSystem>();
		transformSystem->Init();

		// 描画システム
		ecsCoordinator.RegisterSystem<ecs::RenderSystem>(_desc);
		const auto& renderSystem = ecsCoordinator.GetSystem<ecs::RenderSystem>();
		renderSystem->SetGraphicsEngine(_engine);
		renderSystem->Init();

		// デバッグ描画システム
		ecsCoordinator.RegisterSystem<ecs::DebugRenderSystem>(_desc);
		const auto& debugRenderSystem = ecsCoordinator.GetSystem<ecs::DebugRenderSystem>();
		debugRenderSystem->SetGraphicsEngine(_engine);
		debugRenderSystem->Init();

	}
}
#pragma endregion

/**
 * @brief コンストラクタ
 * @param _desc ゲームの定義
 */
dx3d::Game::Game(const GameDesc& _desc)
	: Base({ *std::make_unique<Logger>(_desc.logLevel).release() }),
	logger_ptr_(&logger_)
{
	// デバッグログ初期化
	debug::Debug::Init(true);


	// 時間初期化
	last_time_ = std::chrono::high_resolution_clock::now();

	// 描画エンジンの生成
	graphics_engine_ = std::make_unique<GraphicsEngine>(GraphicsEngineDesc{ logger_ });

	// ウィンドウの生成
	display_ = std::make_unique<Display>(DisplayDesc{ {logger_, _desc.windowSize}, graphics_engine_->GetGraphicsDevice() });
	try {
		// ImGuiの初期化
		ID3D11Device* device = graphics_engine_->GetGraphicsDevice().GetD3DDevice().Get();
		ID3D11DeviceContext* context = graphics_engine_->GetDeviceContext().GetDeviceContext().Get();
		void* hwnd = display_->GetHandle();
		debug::DebugUI::Init(device, context, hwnd);

		// InputSystem初期化
		input::InputSystem::Get().Init(static_cast<HWND>(display_->GetHandle()));
		// ECSのコーディネーターの生成
		ecs_coordinator_ = std::make_unique<ecs::Coordinator>(dx3d::BaseDesc{ logger_ });
		ecs_coordinator_->Init();

		// SceneManagerの初期化
		scene_manager_ = std::make_unique<scene::SceneManager>(scene::SceneManagerDesc{ {logger_}, *ecs_coordinator_ });

		// todo: 自動でComponentを登録する機能が欲しいかも。
		ecs_coordinator_->RegisterComponent<ecs::Transform>();
		ecs_coordinator_->RegisterComponent<ecs::MeshRenderer>();
		ecs_coordinator_->RegisterComponent<ecs::Camera>();
		ecs_coordinator_->RegisterComponent<ecs::CameraController>();
		ecs_coordinator_->RegisterComponent<ecs::Collider>();
		ecs_coordinator_->RegisterComponent<ecs::Rigidbody>();

		// Systemの登録
		ecs::SystemDesc systemDesc{ {logger_ }, *ecs_coordinator_ };
		RegisterAllSystems(systemDesc, *graphics_engine_);

		// Sceneの生成・読み込み・アクティベート
		scene_manager_->ChangeScene("TestScene");

		// Entityの生成

		//// テスト
		//{
		//	auto e = ecs_coordinator_->CreateEntity();
		//	ecs_coordinator_->AddComponent<ecs::Transform>(e, {});
		//	auto handle = graphics_engine_->GetMeshRegistry().GetHandleByName("Cube");
		//	ecs_coordinator_->AddComponent<ecs::MeshRenderer>(e, { handle });
		//	ecs::Collider col{
		//		.type = collision::ShapeType::Box,
		//		.shape = collision::BoxShape{}
		//	};
		//	ecs_coordinator_->AddComponent<ecs::Collider>(e, col);
		//	ecs_coordinator_->AddComponent<ecs::Rigidbody>(e, {});
		//	scene_manager_->AddEntityToScene(*scene_manager_->GetActiveScene(), e);
		//}

	}
	catch (const std::exception& _e) {
		OutputDebugStringA(("[Init] exception: " + std::string(_e.what()) + "\n").c_str());
		__debugbreak();
	}
	catch (...) {
		OutputDebugStringA("[Init] unknown exception\n");
		__debugbreak();
	}

	DX3DLogInfo("ゲーム開始");
}

dx3d::Game::~Game()
{
	// ImGuiの破棄
	debug::DebugUI::DisposeUI();
	DX3DLogInfo("ゲーム終了");

	debug::Debug::Log(debug::Debug::LogLevel::LOG_INFO, "ゲーム終了処理中...");
	debug::Debug::Shutdown();
}


/**
 * @brief 更新
 */
void dx3d::Game::OnInternalUpdate()
{
	const auto& debugRenderSystem = ecs_coordinator_->GetSystem<ecs::DebugRenderSystem>();
	// 入力の更新
	input::InputSystem::Get().Update();
	dx3d::Point mouseDelta = input::InputSystem::Get().GetMouseDelta();

	// 時間の更新
	using clock = std::chrono::high_resolution_clock;
	auto now = clock::now();
	std::chrono::duration<float> delta = now - last_time_;
	float dt = delta.count();	// 秒
	last_time_ = now;

	// 描画前処理
	// スワップチェインのセット
	graphics_engine_->SetSwapChain(display_->GetSwapChain());
	graphics_engine_->BeginFrame();

	if (input::InputSystem::Get().IsKeyTrigger('T'))
	{
		scene_manager_->SaveActiveScene();
	}
	if (input::InputSystem::Get().IsKeyTrigger('1'))
	{
		scene_manager_->ChangeScene("TestScene");
	}
	if (input::InputSystem::Get().IsKeyTrigger('2'))
	{
		scene_manager_->ChangeScene("TestScene2");
	}



	// Systemの更新
	accumulated_time_ += dt;
	while (accumulated_time_ >= fixed_time_step_) {
		ecs_coordinator_->FixedUpdateAllSystems(fixed_time_step_);
		accumulated_time_ -= fixed_time_step_;
	}
	ecs_coordinator_->UpdateAllSystems(dt);

	// デバッグUIの描画
	debug::DebugUI::Render();

	// 描画
	graphics_engine_->EndFrame();
}

