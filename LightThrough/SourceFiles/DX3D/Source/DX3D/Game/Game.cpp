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
	LightThrough::InputSystem::Get().AddListener(this);
	// ウィンドウの生成
	display_ = std::make_unique<Display>(DisplayDesc{ {logger_, _desc.windowSize}, graphics_engine_->GetGraphicsDevice() });

	DX3DLogInfo("ゲーム開始");
}

dx3d::Game::~Game()
{
	// インプットシステムから削除
	LightThrough::InputSystem::Get().RemoveListener(this);
	DX3DLogInfo("ゲーム終了");
}


/**
 * @brief 更新
 */
void dx3d::Game::OnInternalUpdate()
{
	// 入力の更新
	LightThrough::InputSystem::Get().Update();

	// 描画
	graphics_engine_->Render(display_->GetSwapChain());
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
