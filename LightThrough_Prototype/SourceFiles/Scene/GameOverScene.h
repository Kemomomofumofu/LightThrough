#pragma once
#include "SceneBase.h"

class GameManager;
class SceneManager;

class GameOverScene :
	public SceneBase
{
public:
	GameOverScene(SceneManager* _sceneManager, GameManager* _gameManager);
	~GameOverScene() override;

	void Init() override;
	void Uninit() override;

	void Update() override;

private:
	class Camera* camera_;
	class Player* player_;
};

