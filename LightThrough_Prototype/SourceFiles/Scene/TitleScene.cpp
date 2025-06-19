
#include "../GameManager.h"

#include "TitleScene.h"
#include "SceneManager.h"

#include "../GameObjects/GameObject/Camera.h"
#include "../GameObjects/GameObject/Player.h"

#include "../GameObjects/Component/TransformComponent.h"

TitleScene::TitleScene(SceneManager* _sceneManager, GameManager* _gameManager)
	:SceneBase(_sceneManager, _gameManager)
{

}

TitleScene::~TitleScene()
{
	this->Uninit();
}

void TitleScene::Init()
{
	if(!camera_ || !background_)
	{
		{	// ƒJƒƒ‰
			camera_ = new Camera(game_manager_);
		}
		{	// ”wŒi
			background_ = new GameObject(game_manager_, "title_background");
			auto background_sprite = new SpriteComponent(background_, "title_background");
			background_->GetTransformComponent()->SetSize({ 720.0f, 720.0f, 0 });

		}
		{	// UI
			UI_PressEnter_ = new GameObject(game_manager_, "UI_PressEnter");
			auto UISprite = new SpriteComponent(UI_PressEnter_, "title_PressEnter");
			UI_PressEnter_->GetTransformComponent()->SetPosition({ 0, -180.0f, 0 });
			UI_PressEnter_->GetTransformComponent()->SetSize({ 480.0f, 48.0f, 0 });
		}
	}

	

}

void TitleScene::Uninit()
{

}

void TitleScene::Update()
{


	auto& input = InputManager::GetInstance();
	if (input.GetKeyTrigger(VK_RETURN))
	{
		GetSceneManager()->ChangeScene(SceneManager::SceneID::Game);
	}
}
