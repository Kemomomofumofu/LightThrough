#pragma once

#include <memory>
#include <vector>

class GameManager;
class SceneManager;
class GameObject;

class SceneBase
{
public:
	SceneBase(SceneManager* _sceneManager, GameManager* _gameManager);
    virtual ~SceneBase();
    virtual void Init() = 0;
	virtual void Uninit() = 0;
    virtual void Update() = 0;
    // ゲームオブジェクト更新
    void UpdateGameObjects(void);
    virtual void FixedUpdateGameObjects();
    // ゲームオブジェクトの追加
    void AddGameObject(GameObject* _gameObject);
    // ゲームオブジェクトの削除
    void RemoveGameObject(GameObject* _gameObject);

	auto GetGameObjects(void) const { return game_objects_; }
	auto GetPendingGameObjects(void) const { return pending_game_objects_; }

	auto GetSceneManager(void) const { return scene_manager_; }

protected:
    // ゲームマネージャ
	GameManager* game_manager_{};
    // シーンの所有者
	SceneManager* scene_manager_{};

    //更新処理

    // オブジェクトが更新中か確認
    bool updating_game_objects_;

    // ゲームオブジェクト
    std::vector<GameObject*> game_objects_{};			// ゲームオブジェクトコンテナ
    std::vector<GameObject*> pending_game_objects_{};	// 待機コンテナ



};
