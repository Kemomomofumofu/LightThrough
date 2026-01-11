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
#include <Game/Systems/Scenes/TitleSceneSystem.h>
#include <Game/Systems/Initialization/Resolve/ObjectResolveSystem.h>
#include <Game/Systems/Initialization/Resolve/MoveDirectionSourceResolveSystem.h>

#include <Game/Components/Render/MeshRenderer.h>
#include <Game/Components/Core/Transform.h>
#include <Game/Components/Camera/Camera.h>
#include <Game/Components/Input/CameraController.h>
#include <Game/Components/Physics/Collider.h>
#include <Game/Components/Physics/Rigidbody.h>
#include <Game/Components/Render/Light.h>
#include <Game/Components/Input/PlayerController.h>
#include <Game/Components/Input/MoveDirectionSource.h>
#include <Game/Components/Core/Name.h>
#include <Game/Components/Core/ObjectRoot.h>
#include <Game/Components/Core/ObjectChild.h>

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
		_ecs.RegisterComponent<ecs::Name>();
		_ecs.RegisterComponent<ecs::ObjectRoot>();
		_ecs.RegisterComponent<ecs::ObjectChild>();
	}
	/**
	 * @brief システムの登録
	 * @param _ecs ECSのコーディネーター
	 */
	void RegisterAllSystems(ecs::SystemDesc& _desc, dx3d::GraphicsEngine& _engine)
	{
		auto& ecs = _desc.ecs;

		// ---------- 初期化関係 ---------- //
		_desc.oneShot = true;
		ecs.RegisterSystem<ecs::ObjectResolveSystem>(_desc);
		ecs.RegisterSystem<ecs::MoveDirectionSourceResolveSystem>(_desc);

		// ---------- ゲーム関係 ---------- //
		_desc.oneShot = false;
		// 入力関係
		ecs.RegisterSystem<ecs::PlayerControllerSystem>(_desc);

		// ---------- 衝突関係 ---------- // 
		ecs.RegisterSystem<ecs::ForceAccumulationSystem>(_desc);
		ecs.RegisterSystem<ecs::CollisionResolveSystem>(_desc);
		ecs.RegisterSystem<ecs::IntegrationSystem>(_desc);
		ecs.RegisterSystem<ecs::ColliderSyncSystem>(_desc);

		ecs.RegisterSystem<ecs::LightDepthRenderSystem>(_desc);
		const auto& lightDepthRenderSystem = ecs.GetSystem<ecs::LightDepthRenderSystem>();
		lightDepthRenderSystem->SetGraphicsEngine(_engine);

		ecs.RegisterSystem<ecs::ShadowTestSystem>(_desc);
		const auto& shadowTest = ecs.GetSystem<ecs::ShadowTestSystem>();
		shadowTest->SetGraphicsEngine(_engine);

		ecs.RegisterSystem<ecs::ClearForcesSystem>(_desc);

		// タイトル独自の更新
		ecs.RegisterSystem<ecs::TitleSceneSystem>(_desc);

		// 位置更新
		ecs.RegisterSystem<ecs::TransformSystem>(_desc);

		// カメラ
		ecs.RegisterSystem<ecs::CameraSystem>(_desc);
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
} // namespace anonymous
#pragma endregion


namespace dx3d {

	/**
	 * @brief コンストラクタ
	 * @param _desc ゲームの定義
	 */
	Game::Game(const GameDesc& _desc)
		: Base({ *std::make_unique<Logger>(_desc.logLevel).release() }),
		logger_ptr_(&logger_)
	{
		// デバッグログ初期化
		debug::Debug::Init(true);

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

			// Sceneの生成・読み込み・アクティベート
			ChangeScene("TestScene");

			// Systemの登録
			ecs::SystemDesc systemDesc{ {logger_ }, *ecs_coordinator_, *scene_manager_ };
			RegisterAllSystems(systemDesc, *graphics_engine_);



			// 時間初期化
			last_time_ = std::chrono::high_resolution_clock::now();

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

	Game::~Game()
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
	void Game::OnInternalUpdate()
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

		if (input::InputSystem::Get().IsKeyTrigger(VK_RETURN))
		{
			scene_manager_->ChangeScene("TestScene");
		}

		// シーンの保存
		if (input::InputSystem::Get().IsKeyTrigger('T'))
		{
			SaveScene();
		}
		// シーンリロード
		if (input::InputSystem::Get().IsKeyTrigger('R')) {
			ReloadScene();
		}
		//if (input::InputSystem::Get().IsKeyTrigger('2'))
		//{
		//	scene_manager_->ChangeScene("TitleScene");
		//}


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

	//! @brief シーンの保存
	void Game::SaveScene()
	{
		if (!scene_manager_) {
			DX3DLogError("SceneManagerが存在しない。");
			return;
		}

		scene_manager_->SaveActiveScene();
	}

	void Game::ReloadScene()
	{
		if (!ecs_coordinator_) {
			DX3DLogError("ECS::Coordinatorが存在しない。");
			return;
		}
		if (!scene_manager_) {
			DX3DLogError("SceneManagerが存在しない。");
			return;
		}
		scene_manager_->ReloadActiveScene();		// シーンリロード
		ecs_coordinator_->ReactivateAllSystems();	// システムの再アクティブ化
		ecs_coordinator_->FlushPending();			// 保留中の変更を反映
		
	}

	void Game::ChangeScene(const scene::SceneData::Id& _newScene)
	{
		scene_manager_->ChangeScene(_newScene);
		ecs_coordinator_->ReactivateAllSystems();	// システムの再アクティブ化
		ecs_coordinator_->FlushPending();			// 保留中の変更を反映
	}

}