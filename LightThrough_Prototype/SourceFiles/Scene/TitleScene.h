#pragma once
#include "SceneBase.h"

class GameManager;
class SceneManager;

class TitleScene :
    public SceneBase
{
public:
	TitleScene(SceneManager* _sceneManager, GameManager* _gameManager);
	~TitleScene() override;

	void Init() override;
	void Uninit() override;

	void Update() override;

private:
	class Camera* camera_;
	class GameObject* background_;
	class GameObject* UI_PressEnter_;
};

