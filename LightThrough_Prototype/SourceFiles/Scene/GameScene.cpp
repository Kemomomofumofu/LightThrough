#include "GameScene.h"

#include "../GameProcess.h"
#include "../GameManager.h"
#include "../InputManager.h"
#include "SceneManager.h"

#include "../GameObjects/GameObject/Camera.h"
#include "../GameObjects/GameObject/Player.h"

using namespace DirectX::SimpleMath;

GameScene::GameScene(SceneManager* _sceneManager, GameManager* _gameManager)
	: SceneBase(_sceneManager, _gameManager)
	, is_game_clear_(false)
{
}

GameScene::~GameScene()
{

}

void GameScene::Init()
{
	{	// カメラ
		camera_ = new Camera(game_manager_);
	}

	{	// ゴール
		goal_ = new GameObject(game_manager_);
		auto transform = goal_->GetTransformComponent();
		transform->SetSize(32.0f, 32.0f);
		transform->SetPosition(240.0f, 300.0f);
		auto sprite = new SpriteComponent(goal_, "door");

		// イベント関係は諦めました
		//collider->OnTriggerEnter = [this](BoxColliderComponent* _other) {
		//	if (_other->GetOwner()->GetType() == GameObject::TypeID::Player) {
		//		this->GameClear();
		//		}
		//	};
	}

	{	// プレイヤー
		player_ = new Player(game_manager_);
		auto transform = player_->GetTransformComponent();
		player_->GetTransformComponent()->SetPosition(-300.0f, -300.0f);
	}

	{	// 地面
		auto block_1 = new GameObject(game_manager_);
		auto transform = block_1->GetTransformComponent();
		transform->SetSize(GameProcess::GetWidth() * 2, 64.0f);
		transform->SetPosition(0.0f, -(GameProcess::GetHeight() * 0.5f));
		auto sprite = new SpriteComponent(block_1, "block");
		sprite->SetTextureRepeat(true, 32.0f);
	}

}
	
void GameScene::Uninit()
{

}

void GameScene::Update()
{
	auto& input = InputManager::GetInstance();
	if (input.GetKeyTrigger(VK_R)) {
		player_->GetTransformComponent()->SetPosition(-300.0f, -300.0f);
	}


}