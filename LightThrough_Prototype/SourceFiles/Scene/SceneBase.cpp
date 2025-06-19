
#include <algorithm>
#include <vector>
#include <iostream>
#include <format>

#include "../GameManager.h"

#include "SceneBase.h"
#include "SceneManager.h"
#include "../GameObjects/GameObject.h"



//-----------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------
SceneBase::SceneBase(SceneManager* _sceneManager, GameManager* _gameManager)
	: scene_manager_(_sceneManager)
	, game_manager_(_gameManager)
	, updating_game_objects_(false)
{
	game_objects_.clear();
	pending_game_objects_.clear();

}

SceneBase::~SceneBase()
{

}


//-----------------------------------------------------------------
// ゲームオブジェクトの総更新処理
//-----------------------------------------------------------------
void SceneBase::UpdateGameObjects(void)
{
	// すべてのゲームオブジェクトの更新
	updating_game_objects_ = true;

	for (auto& gameObject : game_objects_)
	{
		gameObject->Update();		// 更新処理
	}
	updating_game_objects_ = false;

	// 待機リストのゲームオブジェクトの操作
	for (auto& pendingGameObject : pending_game_objects_)
	{
		pendingGameObject->Update();
		game_objects_.emplace_back(pendingGameObject);
	}
	pending_game_objects_.clear();

	// ゲームオブジェクトが破棄状態かチェック
	std::vector<GameObject*> dead_game_objects;
	for (auto& gameObject : game_objects_)
	{
		if (gameObject->GetState() == GameObject::State::Dead)
		{
			delete gameObject;
		}
	}
}

void SceneBase::FixedUpdateGameObjects()
{
	for (auto& gameObject : game_objects_) {
		//gameObject->FixedUpdate();
	}
}


//-----------------------------------------------------------------
// ゲームオブジェクトの追加処理
//-----------------------------------------------------------------
void SceneBase::AddGameObject(GameObject* gameObject)
{
	// ゲームオブジェクトの更新中かで登録先を変更
	if (updating_game_objects_)
	{
		pending_game_objects_.emplace_back(gameObject);	// 待機コンテナ
	}
	else
	{
		game_objects_.emplace_back(gameObject);			// 稼働コンテナ
	}
}

//-----------------------------------------------------------------
// ゲームオブジェクトの削除処理
//-----------------------------------------------------------------
/*
* @param	削除するゲームオブジェクト
* @brief	コンテナの中から削除するオブジェクトを探して削除する
*/
void SceneBase::RemoveGameObject(GameObject* _gameObject) {
	// 待機コンテナ
	auto iter = std::find_if(
		pending_game_objects_.begin(),
		pending_game_objects_.end(),
		[_gameObject](const GameObject* obj) {return obj == _gameObject;}
	);
	if (iter != pending_game_objects_.end())
	{
		pending_game_objects_.erase(iter);
	}

	// 稼働コンテナ
	iter = std::find_if(game_objects_.begin(),
		game_objects_.end(),
		[_gameObject](const GameObject* obj) {return obj == _gameObject;}
	);
	if (iter != game_objects_.end())
	{
		game_objects_.erase(iter);
	}

}
