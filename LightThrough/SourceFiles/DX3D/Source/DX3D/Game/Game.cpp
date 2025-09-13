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

/**
 * @brief コンストラクタ
 * @param _desc ゲームの定義
 */
dx3d::Game::Game(const GameDesc& _desc)
	: Base({ *std::make_unique<Logger>(_desc.logLevel).release() }),
	logger_ptr_(&logger_)
{
	// 描画システムの生成
	graphics_engine_ = std::make_unique<GraphicsEngine>(GraphicsEngineDesc{ logger_ });
	// インプットシステムに登録
	input::InputSystem::Get().AddListener(this);
	// ECSのコーディネーターの生成
	ecs_coordinator_ = std::make_unique<ecs::Coordinator>();
	ecs_coordinator_->Init();

	// ウィンドウの生成
	display_ = std::make_unique<Display>(DisplayDesc{ {logger_, _desc.windowSize}, graphics_engine_->GetGraphicsDevice() });
	// 時間初期化
	last_time_ = std::chrono::high_resolution_clock::now();

	// [ToDo] テスト用で動かしてみる
	// [ToDo] 自動でComponentを登録する機能が欲しいかも。
	ecs_coordinator_->RegisterComponent<ecs::Transform>();
	ecs_coordinator_->RegisterComponent<ecs::Mesh>();
	ecs_coordinator_->RegisterComponent<ecs::Camera>();
	ecs_coordinator_->RegisterComponent<ecs::CameraController>();

	//ecs_coordinator_->RegisterSystem<ecs::MovementSystem>();

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
	ecs_coordinator_->AddComponent<ecs::Camera>(eCamera, {});
	ecs_coordinator_->AddComponent<ecs::CameraController>(eCamera, {});

	//auto e = ecs_coordinator_->CreateEntity();
	//ecs_coordinator_->AddComponent<ecs::Transform>(e, ecs::Transform{ {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} });
	//auto cubeMesh = CreateCube(graphics_engine_->GetGraphicsDevice());
	//ecs_coordinator_->AddComponent<ecs::Mesh>(e, cubeMesh);


	DX3DLogInfo("ゲーム開始");
}

dx3d::Game::~Game()
{
	// インプットシステムから削除
	input::InputSystem::Get().RemoveListener(this);
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

	// 時間の更新
	using clock = std::chrono::high_resolution_clock;
	auto now = clock::now();
	std::chrono::duration<float> delta = now - last_time_;
	float dt = delta.count();	// 秒
	last_time_ = now;

	ecs::Transform testTransform;
	debugRenderSystem->DrawCube({ {0, 0, 0}, { 10, 10, 5 }, {1, 1, 1} }, {1, 1, 1, 0.1f});

	// 描画前処理
	// スワップチェインのセット
	graphics_engine_->SetSwapChain(display_->GetSwapChain());
	graphics_engine_->BeginFrame();

	// Systemの更新
	ecs_coordinator_->UpdateAllSystems(dt);

	// 描画
	graphics_engine_->EndFrame();
}

void dx3d::Game::OnKeyDown(int _key)
{
	if (_key == 'W') {
		DX3DLogInfo("Wキーが押された");
	}
}

void dx3d::Game::OnKeyUp(int _key)
{

}

void dx3d::Game::OnMouseMove(const dx3d::Point& _deltaMousePos)
{

}
