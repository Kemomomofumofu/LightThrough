//==================================================
// [Player.cpp] プレイヤーオブジェクト
// 著者：有馬啓太
//--------------------------------------------------
// 説明：プレイヤーの処理を定義
//==================================================

/*----- インクルード -----*/
#include <iostream>
#include <format>

#include "../../GameManager.h"	
#include "../../Scene/SceneManager.h"

#include "Player.h"
#include "../IComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/RenderComponent/SpriteComponent.h"+

//--------------------------------------------------
// コンストラクタ
//--------------------------------------------------
Player::Player(GameManager* _gameManager)
	:GameObject(_gameManager, "Player")
{
	// スプライトコンポーネント
	sprite_component_ = new SpriteComponent(this, "yurei");

	InitGameObject();
}

//--------------------------------------------------
// デストラクタ
//--------------------------------------------------
Player::~Player(void)
{

}
	
//--------------------------------------------------
// 初期化処理
//--------------------------------------------------
void Player::InitGameObject(void)
{
	transform_component_->SetSize(32.0f, 32.0f);

}

//--------------------------------------------------
// 更新処理
//--------------------------------------------------
void Player::UpdateGameObject(void)
{

}

