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

#include <Game/Systems/TransformSystem.h>
#include <Game/Systems/CameraSystem.h>
#include <Game/Systems/Gimmicks/ShadowTestSystem.h>
#include <Game/Systems/Renderers/RenderSystem.h>
#include <Game/Systems/Renderers/LightDepthRenderSystem.h>
#include <Game/Systems/Renderers/DebugRenderSystem.h>
#include <Game/Systems/Collisions/ColliderSyncSystem.h>
#include <Game/Systems/Collisions/CollisionResolveSystem.h>
#include <Game/Systems/Physics/ForceAccumulationSystem.h>
#include <Game/Systems/Physics/IntegrationSystem.h>
#include <Game/Systems/Physics/ClearForcesSystem.h>
#include <Game/Systems/PlayerControllerSystem.h>

#include <Game/Components/MeshRenderer.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>
#include <Game/Components/Collider.h>
#include <Game/Components/Physics/Rigidbody.h>
#include <Game/Components/Light.h>
#include <Game/Components/PlayerController.h>
#include <Game/Components/MoveDirectionSource.h>

#include <Debug/DebugUI.h>
#include <Debug/Debug.h>

#pragma region ローカルメソッド
namespace {

	/**
	 * @brief コンポーネントの登録
	 * @param _ecs ECSのコーディネーター
	 */
	void RegisterAllComponents(ecs::Coordinator& _ecs)
	{
		// todo: 自動でComponentを登録する機能が欲しいかも。システム登録段階で、コンポーネントマネージャの中にすでに登録してあったらそのシグネチャを使い、なければ新たにとか...
		_ecs.RegisterComponent<ecs::Transform>();
		_ecs.RegisterComponent<ecs::MeshRenderer>();
		_ecs.RegisterComponent<ecs::Camera>();
		_ecs.RegisterComponent<ecs::CameraController>();
		_ecs.RegisterComponent<ecs::Collider>();
		_ecs.RegisterComponent<ecs::Rigidbody>();
		_ecs.RegisterComponent<ecs::LightCommon>();
		_ecs.RegisterComponent<ecs::SpotLight>();
		_ecs.RegisterComponent<ecs::PlayerController>();
		_ecs.RegisterComponent<ecs::MoveDirectionSource>();
	}
	/**
	 * @brief システムの登録
	 * @param _ecs ECSのコーディネーター
	 */
	void RegisterAllSystems(ecs::SystemDesc& _desc, dx3d::GraphicsEngine& _engine)
	{
		auto& ecs = _desc.ecs;


		// 入力関係
		ecs.RegisterSystem<ecs::PlayerControllerSystem>(_desc);

		// ---------- 衝突関係 ---------- // 
		ecs.RegisterSystem<ecs::ForceAccumulationSystem>(_desc);
		ecs.RegisterSystem<ecs::IntegrationSystem>(_desc);
		ecs.RegisterSystem<ecs::ColliderSyncSystem>(_desc);

		ecs.RegisterSystem<ecs::LightDepthRenderSystem>(_desc);
		const auto& lightDepthRenderSystem = ecs.GetSystem<ecs::LightDepthRenderSystem>();
		lightDepthRenderSystem->SetGraphicsEngine(_engine);

		ecs.RegisterSystem<ecs::ShadowTestSystem>(_desc);
		const auto& shadowTest = ecs.GetSystem<ecs::ShadowTestSystem>();
		shadowTest->SetGraphicsEngine(_engine);

		ecs.RegisterSystem<ecs::CollisionResolveSystem>(_desc);
		ecs.RegisterSystem<ecs::ClearForcesSystem>(_desc);

		// カメラ
		ecs.RegisterSystem<ecs::CameraSystem>(_desc);
		// 位置更新
		ecs.RegisterSystem<ecs::TransformSystem>(_desc);

		// 描画システム
		ecs.RegisterSystem<ecs::RenderSystem>(_desc);
		const auto& renderSystem = ecs.GetSystem<ecs::RenderSystem>();
		renderSystem->SetGraphicsEngine(_engine);

		// デバッグ描画システム
		ecs.RegisterSystem<ecs::DebugRenderSystem>(_desc);
		const auto& debugRenderSystem = ecs.GetSystem<ecs::DebugRenderSystem>();
		debugRenderSystem->SetGraphicsEngine(_engine);

		// 全システム初期化
		ecs.InitAllSystems();
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
		ID3D11DeviceContext* context = graphics_engine_->GetDeferredContext().GetDeferredContext().Get();
		void* hwnd = display_->GetHandle();
		debug::DebugUI::Init(device, context, hwnd);

		// InputSystem初期化
		input::InputSystem::Get().Init(static_cast<HWND>(display_->GetHandle()));
		// ECSのコーディネーターの生成
		ecs_coordinator_ = std::make_unique<ecs::Coordinator>(dx3d::BaseDesc{ logger_ });
		ecs_coordinator_->Init();

		// SceneManagerの初期化
		scene_manager_ = std::make_unique<scene::SceneManager>(scene::SceneManagerDesc{ {logger_}, *ecs_coordinator_ });

		// Componentの登録
		RegisterAllComponents(*ecs_coordinator_);
		// Systemの登録
		ecs::SystemDesc systemDesc{ {logger_ }, *ecs_coordinator_ };
		RegisterAllSystems(systemDesc, *graphics_engine_);

		// Sceneの生成・読み込み・アクティベート
		scene_manager_->ChangeScene("TestScene");

		// Entityの生成
		// テスト

		//{
		//	auto e = ecs_coordinator_->CreateEntity();
		//	ecs::Transform tf{ {0.0f, 20.0f, 0.0f} };
		//	tf.LookTo({ 0.0f, -1.0f, 0.0f });
		//	ecs_coordinator_->AddComponent<ecs::Transform>(e, tf);
		//	ecs::LightCommon lightCommon;
		//	lightCommon.color = { 0.7f, 0.5f, 0.6f };
		//	ecs_coordinator_->AddComponent<ecs::LightCommon>(e, lightCommon);
		//	scene_manager_->AddEntityToScene(*scene_manager_->GetActiveScene(), e);
		//}

		//{
		//	auto e = ecs_coordinator_->CreateEntity();
		//	ecs::Transform tf{ {0.0f, 20.0f, 0.0f} };
		//	tf.LookTo({ 0.0f, -1.0f, 0.0f });
		//	ecs_coordinator_->AddComponent<ecs::Transform>(e, tf);
		//	ecs::LightCommon lightCommon;
		//	lightCommon.color = { 0.0f, 0.95f, 0.2f };
		//	ecs_coordinator_->AddComponent<ecs::LightCommon>(e, lightCommon);
		//	ecs::SpotLight spotData{};
		//	spotData.range = 100.0f;
		//	spotData.innerCos = 0.9f;
		//	spotData.outerCos = 0.8f;
		//	ecs_coordinator_->AddComponent<ecs::SpotLight>(e, spotData);
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
	ecs_coordinator_->FlushPending();

	// デバッグUIの描画
	debug::DebugUI::Render();

	// 描画
	graphics_engine_->EndFrame();

#ifdef defined(_DEBUG) || defined(DEBUG)

	if (auto* dev = graphics_engine_->GetGraphicsDevice().GetD3DDevice().Get()) {
		const HRESULT hr = dev->GetDeviceRemovedReason();
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET || FAILED(hr)) {
			char buf[256]{};
			sprintf_s(buf, "DeviceRemovedReason=0x%08X\n", static_cast<unsigned>(hr));
			OutputDebugStringA(buf);
			__debugbreak();
		}
	}
#endif // _DEBUG || DEBUG

}

