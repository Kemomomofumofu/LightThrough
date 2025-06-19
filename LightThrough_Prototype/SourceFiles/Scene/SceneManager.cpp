#include <iostream>
#include "../GameManager.h"
#include "../Renderer.h"
#include "SceneManager.h"
#include "SceneBase.h"

#include "TitleScene.h"
#include "GameScene.h"
#include "ClearScene.h"
#include "GameOverScene.h"

SceneManager::SceneManager(GameManager* _gameManager)
	: game_manager_(_gameManager)
    ,current_scene_(nullptr)
{

}

SceneManager::~SceneManager()
{
    Uninit();
}

void SceneManager::Init()
{
    current_scene_ = std::make_unique<TitleScene>(this, game_manager_);
    // 初期シーン
	current_scene_->Init();
}

void SceneManager::Uninit()
{
	ClearAllObjects();

}

void SceneManager::Update()
{
	current_scene_->Update();
    current_scene_->UpdateGameObjects();
}




void SceneManager::ChangeScene(SceneID _newScene)
{
	// 現在のシーンの終了処理
	if (current_scene_) {
		try {
			current_scene_->Uninit();
		}
		catch (const std::exception& e) {
			std::cerr << "[ChangeScene] -> 例外 : " << e.what() << std::endl;
		}
	}

	// ゲームオブジェクトの全削除
	try {
		ClearAllObjects();
	}
	catch (const std::exception& e) {
		std::cerr << "[ClearAllObjects] -> 例外 : " << e.what() << std::endl;
	}

    switch (_newScene)
    {
	case SceneID::Title:
		current_scene_ = std::make_unique<TitleScene>(this, game_manager_);
		break;
	case SceneID::Game:
		current_scene_ = std::make_unique<GameScene>(this, game_manager_);
		break;
	case SceneID::Clear:
		current_scene_ = std::make_unique<ClearScene>(this, game_manager_);
        break;
	case SceneID::GameOver:
		current_scene_ = std::make_unique<GameOverScene>(this, game_manager_);
		break;
    default:
        break;
    }

    // シーンの初期化
	current_scene_->Init();
}


void SceneManager::ClearAllObjects(void)
{
	// 削除対象リストを作成
	std::vector<GameObject*> dead_game_objects;

	// 稼働中のゲームオブジェクトを削除対象リストに追加
	for (auto& obj : current_scene_->GetGameObjects()) {
		dead_game_objects.push_back(obj);
	}
	// 待機中のゲームオブジェクトを削除対象リストに追加
	for (auto& obj : current_scene_->GetPendingGameObjects()) {
		dead_game_objects.push_back(obj);
	}

	// 削除対象リスト内のオブジェクトを安全に削除
	for (auto& obj : dead_game_objects) {
		delete obj;
	}

	// ゲームオブジェクトリストをクリア
	current_scene_->GetGameObjects().clear();
	current_scene_->GetPendingGameObjects().clear();
}
