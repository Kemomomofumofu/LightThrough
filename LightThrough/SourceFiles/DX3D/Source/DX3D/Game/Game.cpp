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
#include <DX3D/Core/Logger.h>
#include <DX3D/Game/Display.h>
#include <DX3D/Math/Point.h>
#include <InputSystem/InputSystem.h>

#include <DX3D/Game/ECS/Components/Transform.h>
#include <DX3D/Game/ECS/Components/Velocity.h>
#include <Game/Systems/MovementSystem.h>
#include <DX3D/Game/ECS/Systems/RenderSystem.h>

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
	ecs_coordinator_->RegisterComponent<ecs::Transform>();
	ecs_coordinator_->RegisterComponent<ecs::Velocity>();

	ecs_coordinator_->RegisterSystem<ecs::MovementSystem>();
	ecs::Signature moveSig;
	moveSig.set(ecs_coordinator_->GetComponentType<ecs::Transform>());
	moveSig.set(ecs_coordinator_->GetComponentType<ecs::Velocity>());
	ecs_coordinator_->SetSystemSignature<ecs::MovementSystem>(moveSig);

	ecs_coordinator_->RegisterSystem<ecs::RenderSystem>();
	ecs::Signature renderSig;
	//renderSig.set(ecs_coordinator_->GetComponentType<ecs::mesh>());
	renderSig.set(ecs_coordinator_->GetComponentType<ecs::Transform>());
	ecs_coordinator_->SetSystemSignature<ecs::RenderSystem>(renderSig);
	const auto& renderSystem = ecs_coordinator_->GetSystem<ecs::RenderSystem>();
	renderSystem->SetGraphicsEngine(*graphics_engine_);

	// Entityの生成
	auto e = ecs_coordinator_->CreateEntity();
	ecs_coordinator_->AddComponent<ecs::Transform>(e, ecs::Transform{ {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} });
	ecs_coordinator_->AddComponent<ecs::Velocity>(e, ecs::Velocity{ {10.0f, 0.0f, 0.0f} });

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
	// 入力の更新
	input::InputSystem::Get().Update();

	// 時間の更新
	using clock = std::chrono::high_resolution_clock;
	auto now = clock::now();
	std::chrono::duration<float> delta = now - last_time_;
	float dt = delta.count();	// 秒
	last_time_ = now;

	// スワップチェインのセット
	graphics_engine_->SetSwapChain(display_->GetSwapChain());
	// Systemの更新
	ecs_coordinator_->UpdateSystems(dt);


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
