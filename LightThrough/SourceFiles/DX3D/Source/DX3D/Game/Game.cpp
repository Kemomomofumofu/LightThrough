/**
 * @file Game.cpp
 * @brief ゲーム
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Game/Game.h>
#include <DX3D/Window/Window.h>
#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Game/Display.h>
#include <DX3D/Math/Point.h>
#include <DX3D/Graphics/Meshes/PrimitiveFactory.h>
#include <Game/Scene/SceneManager.h>
#include <Game/InputSystem/InputSystem.h>
#include <Game/Serialization/ComponentReflections.h>
#include <Game/Serialization/ComponentReflection.h>

#include <Game/Systems/Factorys/PrefabSystem.h>
#include <Game/Systems/TransformSystem.h>
#include <Game/Systems/MovementSystem.h>
#include <Game/Systems/RenderSystem.h>
#include <Game/Systems/CameraSystem.h>
#include <Game/Systems/DebugRenderSystem.h>
#include <Game/Systems/Scenes/TitleSceneSystem.h>
#include <Game/Systems/Collisions/ColliderSyncSystem.h>
#include <Game/Systems/Collisions/CollisionResolveSystem.h>

#include <Game/Components/MeshRenderer.h>
#include <Game/Components/Transform.h>
//#include <Game/Components/Velocity.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>
#include <Game/Components/Collider.h>

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

		ecsCoordinator.RegisterSystem<ecs::TransformSystem>(_desc);
		const auto& transformSystem = ecsCoordinator.GetSystem<ecs::TransformSystem>();
		transformSystem->Init();

		// コライダー同期システム
		ecsCoordinator.RegisterSystem<ecs::ColliderSyncSystem>(_desc);
		const auto& colliderSyncSystem = ecsCoordinator.GetSystem<ecs::ColliderSyncSystem>();
		colliderSyncSystem->Init();

		ecsCoordinator.RegisterSystem<ecs::CollisionResolveSystem>(_desc);
		const auto& collisionResolveSystem = ecsCoordinator.GetSystem<ecs::CollisionResolveSystem>();
		collisionResolveSystem->Init();

		// カメラシステム
		ecsCoordinator.RegisterSystem<ecs::CameraSystem>(_desc);
		const auto& cameraSystem = ecsCoordinator.GetSystem<ecs::CameraSystem>();
		cameraSystem->Init();

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

	// [ToDo] テスト用で動かしてみる
	// [ToDo] 自動でComponentを登録する機能が欲しいかも。
	ecs_coordinator_->RegisterComponent<ecs::Transform>();
	ecs_coordinator_->RegisterComponent<ecs::MeshRenderer>();
	ecs_coordinator_->RegisterComponent<ecs::Camera>();
	ecs_coordinator_->RegisterComponent<ecs::CameraController>();
	ecs_coordinator_->RegisterComponent<ecs::Collider>();
	// デシリアライズハンドラ登録
	using Coord = ecs::Coordinator;
	using Ent = ecs::Entity;
	REGISTER_COMPONENT_DESERIALIZER(Coord, Ent, ecs::Transform);
	REGISTER_COMPONENT_DESERIALIZER(Coord, Ent, ecs::MeshRenderer);
	REGISTER_COMPONENT_DESERIALIZER(Coord, Ent, ecs::Camera);
	REGISTER_COMPONENT_DESERIALIZER(Coord, Ent, ecs::CameraController);
	REGISTER_COMPONENT_DESERIALIZER(Coord, Ent, ecs::Collider);

	// Systemの登録
	ecs::SystemDesc systemDesc{ {logger_ }, *ecs_coordinator_ };
	RegisterAllSystems(systemDesc, *graphics_engine_);

	// Sceneの生成・読み込み・アクティベート
	scene_manager_->ChangeScene("TestScene");

	// Entityの生成

	// テスト
	for (int i = 0; i < 2; ++i) {
		auto e = ecs_coordinator_->CreateEntity();
		ecs_coordinator_->AddComponent<ecs::Transform>(e, ecs::Transform{ {1.5f * i, 0.0f, 0.0f} });
		auto& tf = ecs_coordinator_->GetComponent<ecs::Transform>(e);
		auto handle = graphics_engine_->GetMeshRegistry().GetHandleByName("Cube");
		ecs_coordinator_->AddComponent<ecs::MeshRenderer>(e, { handle });
		ecs::Collider col {
		.type = collision::ShapeType::Box,
		.shape = collision::BoxShape{ {0.5f, 0.5f, 0.5f} },
		.isStatic = false,
		};
		ecs_coordinator_->AddComponent<ecs::Collider>(e, col);
		scene_manager_->AddEntityToScene(*scene_manager_->GetActiveScene(), e);
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
	ecs_coordinator_->UpdateAllSystems(dt);

	// デバッグUIの描画
	debug::DebugUI::Render();

	// 描画
	graphics_engine_->EndFrame();
}

