//=================================================================
// [GameScene.h] ゲームシーン
// 著者：有馬啓太
//-----------------------------------------------------------------
// 説明：ゲームをプレイするシーン
//=================================================================
#ifndef GAME_SCENE_H_
#define GAME_SCENE_H_
/*----- インクルード -----*/
#include "SceneBase.h"
/*----- 構造体定義 -----*/

/*----- 前方宣言 -----*/

//-----------------------------------------------------------------
// GameScene
//-----------------------------------------------------------------
class GameScene :
    public SceneBase
{
public:
	GameScene(SceneManager* _sceneManager, GameManager* _gameManager);
	~GameScene() override;
    virtual void Init();
    virtual void Uninit();
    virtual void Update();

	void GameClear(void) { is_game_clear_ = true; }

private:
	class Camera* camera_{};
	class Player* player_{};
	class GameObject* goal_{};

private:
	bool is_game_clear_{};
};


#endif GAME_SCENE_H_




