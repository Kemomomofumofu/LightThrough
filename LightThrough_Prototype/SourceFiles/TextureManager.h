
//=================================================================
// [TextureManager.h] テクスチャ管理クラス
// 著者：有馬啓太
//-----------------------------------------------------------------
// 説明：テクスチャのリソース管理を行うクラス
//		 シングルトンでの実装です
//=================================================================
#ifndef TEXTURE_MANAGER_H_
#define TEXTURE_MANAGER_H_

#define TEXTURE_PATH "Assets/Texture/"


/*----- インクルード -----*/
#include <unordered_map>
#include <string>
#include <wrl/client.h>
#include <d3d11.h>
#include <memory>

/*----- 構造体定義 -----*/

/*----- 前方宣言 -----*/
class Texture;


//-----------------------------------------------------------------
// テクスチャマネージャ 
//-----------------------------------------------------------------
class TextureManager
{
public:
	// インスタンス取得
	static TextureManager& GetInstance()
	{
		static TextureManager instance;
		return instance;
	}

	void Init();
	void Uninit();

	// テクスチャの登録
	void RegisterTexture(const std::string& _imgName, const std::string& _fileName, int _u = 1, int _v = 1, bool _roopFlg = false);
	// テクスチャの取得
	std::shared_ptr<Texture> GetTexture(const std::string& _imgName);


private:
	// シングルトンなのでprivateに
	TextureManager();
	~TextureManager();

	// テクスチャの全登録
	void RegisterAllTextures();
	// 解放処理
	void ClearCache(void);

	// テクスチャキャッシュ
	std::unordered_map<std::string, std::shared_ptr<Texture>> texture_cache_;

};


#endif TEXTURE_MANAGER_H_