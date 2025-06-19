//=================================================================
// [GameManager.h] ゲームマネージャーモジュールヘッダ
// 著者：有馬啓太
//-----------------------------------------------------------------
// 説明：ゲームの状態やオブジェクトを管理するためのクラス
//=================================================================
#ifndef GAME_MANAGER_H_
#define GAME_MANAGER_H_


/*----- インクルード -----*/
#include<memory>
#include<vector>

/*----- 構造体定義 -----*/

/*----- 前方宣言 -----*/
class GameObject;

//-----------------------------------------------------------------
// ゲームマネージャークラス
//-----------------------------------------------------------------
class GameManager
{
public:
	GameManager(void);
	~GameManager(void);

	// インスタンス生成
	static auto Create(void) { return new GameManager(); }

	void InitAll(void);
	void UninitAll(void);
	void UpdateAll(void);
	void GenerateOutputAll(void);

	auto GetRenderer()const { return renderer_.get(); }
	auto GetSceneManager()const { return scene_manager_.get(); }

private:
	// 先に宣言したもの程後で解放されるみたいです
	std::unique_ptr<class Renderer> renderer_;
	std::unique_ptr<class SceneManager> scene_manager_;
};


#endif //GAME_MANAGER_H_

