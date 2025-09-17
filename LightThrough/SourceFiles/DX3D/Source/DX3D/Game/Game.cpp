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
#include <DX3D/Game/Display.h>
#include <DX3D/Math/Point.h>
#include <DX3D/Graphics/PrimitiveFactory.h>
//#include <DX3D/Game/Scene/SceneManager.h>
#include <InputSystem/InputSystem.h>

#include <Game/Systems/MovementSystem.h>
#include <Game/Systems/RenderSystem.h>
#include <Game/Systems/CameraSystem.h>
#include <Game/Systems/DebugRenderSystem.h>

#include <Game/Components/Mesh.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Velocity.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>

#include <DX3D/Game/ECS/ECSLogUtils.h>
#include <Debug/DebugUI.h>


/**
 * @brief コンストラクタ
 * @param _desc ゲームの定義
 */
dx3d::Game::Game(const GameDesc& _desc)
	: Base({ *std::make_unique<Logger>(_desc.logLevel).release() }),
	logger_ptr_(&logger_)
{

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
	ecs_coordinator_ = std::make_unique<ecs::Coordinator>();
	ecs_coordinator_->Init();

	//// SceneManagerの初期化
	//scene_manager_ = std::make_unique<scene::SceneManager>(BaseDesc{ logger_ });
	//scene_manager_->Init(*ecs_coordinator_);

	// [ToDo] テスト用で動かしてみる
	// [ToDo] 自動でComponentを登録する機能が欲しいかも。
	ecs_coordinator_->RegisterComponent<ecs::Transform>();
	ecs_coordinator_->RegisterComponent<ecs::Mesh>();
	ecs_coordinator_->RegisterComponent<ecs::Camera>();
	ecs_coordinator_->RegisterComponent<ecs::CameraController>();

	//// Sceneの生成・読み込み・アクティベート
	//auto sceneId = scene_manager_->CreateScene("TestScene");
	//scene_manager_->LoadSceneFromFile("Assets/Scenes/TestScene.json", sceneId);
	//scene_manager_->SetActiveScene(sceneId, false);


	// SystemDescの準備
	dx3d::SystemDesc systemDesc{ logger_ };

	// 描画システム
	ecs_coordinator_->RegisterSystem<ecs::RenderSystem>(systemDesc);
	const auto& renderSystem = ecs_coordinator_->GetSystem<ecs::RenderSystem>();
	renderSystem->SetGraphicsEngine(*graphics_engine_);
	renderSystem->Init(*ecs_coordinator_);
	// カメラシステム
	ecs_coordinator_->RegisterSystem<ecs::CameraSystem>(systemDesc);
	const auto& cameraSystem = ecs_coordinator_->GetSystem<ecs::CameraSystem>();
	cameraSystem->Init(*ecs_coordinator_);
	// デバッグ描画システム
	ecs_coordinator_->RegisterSystem<ecs::DebugRenderSystem>(systemDesc);
	const auto& debugRenderSystem = ecs_coordinator_->GetSystem<ecs::DebugRenderSystem>();
	debugRenderSystem->SetGraphicsEngine(*graphics_engine_);
	debugRenderSystem->Init(*ecs_coordinator_);

	// Entityの生成
	// カメラ
	auto eCamera = ecs_coordinator_->CreateEntity();
	ecs_coordinator_->AddComponent<ecs::Transform>(eCamera, ecs::Transform{ {0.0f, 0.0f, -5.0f}, {0.0f, 0.0f, 0.0f} });
	ecs_coordinator_->AddComponent<ecs::Camera>(eCamera, ecs::Camera{});
	ecs_coordinator_->AddComponent<ecs::CameraController>(eCamera, ecs::CameraController{ecs::CameraMode::FPS});

	// テストのメッシュ
	auto e = ecs_coordinator_->CreateEntity();
	ecs_coordinator_->AddComponent<ecs::Transform>(e, ecs::Transform{ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} });
	auto mesh = dx3d::PrimitiveFactory::CreateCube(graphics_engine_->GetGraphicsDevice());
	ecs_coordinator_->AddComponent<ecs::Mesh>(e, mesh);



	DX3DLogInfo("ゲーム開始");
}

dx3d::Game::~Game()
{
	debug::DebugUI::DisposeUI();
	DX3DLogInfo("ゲーム終了");
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

	// Systemの更新
	ecs_coordinator_->UpdateAllSystems(dt);

	// デバッグUIの描画
	debug::DebugUI::Render();

	// 描画
	graphics_engine_->EndFrame();
}
