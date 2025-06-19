#pragma once
#include <vector>
#include <memory>

#include "SceneBase.h"
#include "../GameObjects/GameObject.h"

class GameManager;

class SceneManager
{
public:
	enum SceneID {
		None = -1
		, Title
		, Game
		, Clear
		, GameOver
		, Max
	};

	SceneManager(GameManager* _gameManager);
	~SceneManager();

	void Init(void);
	void Uninit(void);
	void Update(void);

	void ClearAllObjects(void);

	void ChangeScene(SceneID _newScene);
	
	SceneBase* GetCurrentScene(void) const { return current_scene_.get(); }
private:
	GameManager* game_manager_{};
	std::unique_ptr<SceneBase> current_scene_{};


};

