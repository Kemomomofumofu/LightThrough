#pragma once
#include "SceneBase.h"

class GameManager;
class SceneManager;

class ClearScene :
	public SceneBase
{
public:
	ClearScene(SceneManager* _sceneManager, GameManager* _gameManager);
	~ClearScene() override;

	void Init() override;
	void Uninit() override;

	void Update() override;

private:
	class Camera* camera_;
};

