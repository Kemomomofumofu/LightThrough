
#include "../GameManager.h"

#include "ClearScene.h"
#include "SceneManager.h"

#include "../GameObjects/GameObject/Camera.h"
#include "../GameObjects/GameObject/Player.h"

#include "../GameObjects/Component/TransformComponent.h"

ClearScene::ClearScene(SceneManager* _sceneManager, GameManager* _gameManager)
	:SceneBase(_sceneManager, _gameManager)
{
	camera_ = new Camera(_gameManager);

}

ClearScene::~ClearScene()
{
	this->Uninit();
}

void ClearScene::Init()
{
	if (!camera_)
	{
		camera_ = new Camera(game_manager_);
	}


	{
		auto UI_clear = new GameObject(game_manager_);
		new SpriteComponent(UI_clear);
	}
}

void ClearScene::Uninit()
{
}

void ClearScene::Update()
{
	auto& input = InputManager::GetInstance();
	if (input.GetKeyTrigger(VK_RETURN))
	{
		GetSceneManager()->ChangeScene(SceneManager::SceneID::Title);
	}
}
