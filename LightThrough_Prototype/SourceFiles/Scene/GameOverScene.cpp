
#include "../GameManager.h"

#include "GameOverScene.h"
#include "SceneManager.h"

#include "../GameObjects/GameObject/Camera.h"
#include "../GameObjects/GameObject/Player.h"

#include "../GameObjects/Component/TransformComponent.h"

GameOverScene::GameOverScene(SceneManager* _sceneManager, GameManager* _gameManager)
	:SceneBase(_sceneManager, _gameManager)
{

}

GameOverScene::~GameOverScene()
{
	this->Uninit();
}

void GameOverScene::Init()
{
	if (!camera_ || !player_)
	{
		camera_ = new Camera(game_manager_);
		player_ = new Player(game_manager_);
	}



	player_->SetState(GameObject::State::Paused);
	player_->GetComponent<TransformComponent>()->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
}

void GameOverScene::Uninit()
{
}

void GameOverScene::Update()
{
	auto& input = InputManager::GetInstance();
	if (input.GetKeyTrigger(VK_RETURN))
	{
		GetSceneManager()->ChangeScene(SceneManager::SceneID::Title);
	}
}
