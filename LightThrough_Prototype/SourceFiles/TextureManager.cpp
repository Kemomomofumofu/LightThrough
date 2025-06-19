//=================================================================
// [TextureManager.cpp] テクスチャ管理クラス
// 著者：有馬啓太
//-----------------------------------------------------------------
// 説明：そんな感じ
//=================================================================

/*----- インクルード -----*/
#include <iostream>
#include "TextureManager.h"
#include "SubSystem/stb_image.h"
#include "SubSystem/Texture.h"
#include "Renderer.h"

//-----------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------
TextureManager::TextureManager()
{
}

//-----------------------------------------------------------------
// デストラクタ
//-----------------------------------------------------------------
TextureManager::~TextureManager()
{
	this->ClearCache();
}

//-----------------------------------------------------------------
// 初期化処理
//-----------------------------------------------------------------
void TextureManager::Init()
{
	RegisterAllTextures();
}

//-----------------------------------------------------------------
// 終了処理
//-----------------------------------------------------------------
void TextureManager::Uninit()
{
	ClearCache();
}

//-----------------------------------------------------------------
// @param  _imgName 画像名, _fileName ファイルパス
// @brief  テクスチャの登録
//-----------------------------------------------------------------
void TextureManager::RegisterTexture(const std::string& _imgName, const std::string& _fileName,int _u, int _v, bool _loopFlg)
{
	// すでに読み込まれているか確認
	auto it = texture_cache_.find(_imgName);
	if (it != texture_cache_.end())
	{
		// すでに読み込まれている
		std::cout << std::format("\n＜TextureManager＞ -> {} is already loaded\n\n", _imgName);
		return;
	}

	// 存在していないので生成する
	auto texture = std::make_shared<Texture>();
	if (texture->Load(_fileName)) {
		texture->SetFrameInfo(_u, _v);
		texture_cache_[_imgName] = texture;
		std::cout << std::format("＜TextureManager＞ -> {} LoadTexture Success\n", _imgName);
		return;
	}

	// 失敗
	std::cout << std::format("＜TextureManager＞ -> {} LoadTexture Error\n", _imgName);
	return;
}

//-----------------------------------------------------------------
// @param  _imgName 画像名
// @brief  読み込んだテクスチャの取得
// @return 存在していればit, なければnullptr
//-----------------------------------------------------------------
std::shared_ptr<Texture> TextureManager::GetTexture(const std::string& _imgName)
{
	auto it = texture_cache_.find(_imgName);
	if (it != texture_cache_.end())
	{
		return it->second;
	}

	std::cout << std::format("＜TextureManager＞ -> {} GetTexture Error\n", _imgName);
	return nullptr;
}

//-----------------------------------------------------------------
// @brief  すべてのテクスチャを登録する
//-----------------------------------------------------------------
void TextureManager::RegisterAllTextures()
{
	// インゲームのオブジェクト系
	/*--------------- ゲームオブジェクト---------------*/
	RegisterTexture("yurei", TEXTURE_PATH"yurei.png");
	RegisterTexture("block", TEXTURE_PATH"block.png");
	RegisterTexture("door", TEXTURE_PATH"door.png");


	// その他の背景、UIなど
	/*--------------- 背景 ---------------*/
	// タイトル
	RegisterTexture("title_background", TEXTURE_PATH"title.png");
	RegisterTexture("title_PressEnter", TEXTURE_PATH"UI_PressEnter.png");

	/*--------------- UI ---------------*/

}

//-----------------------------------------------------------------
// @brief キャッシュのクリア
//-----------------------------------------------------------------
void TextureManager::ClearCache(void)
{
	texture_cache_.clear();
}
