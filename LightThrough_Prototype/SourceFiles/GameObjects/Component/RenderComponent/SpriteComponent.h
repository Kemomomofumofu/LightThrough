//==================================================
// [SpriteComponent.h] 描画コンポーネント
// 著者：有馬啓太
//--------------------------------------------------
// 説明：描画コンポーネントの宣言
//==================================================
#ifndef SPRITE_COMPONENT_H_
#define SPRITE_COMPONENT_H_

/*----- インクルード -----*/
#include <memory>
#include <string>

#include "RenderComponent.h"
#include "../../../SubSystem/Texture.h"



/*----- 構造体定義 -----*/

/*----- 前方宣言 -----*/
class GameObject;

//--------------------------------------------------
// 描画クラス
//--------------------------------------------------
class SpriteComponent :
	public RenderComponent
{
public:
	SpriteComponent(GameObject* _owner, const std::string _imgname = "default", int _drawOrder = 10);
	~SpriteComponent();

	void Init() override;
	void Update() override;
	void Uninit() override;
	void Draw();

	// テクスチャを変更
	void SetTexture(const std::string _imgname);
	// 
	void SetAnimationFrame(int _frameX, int _frameY);
	// テクスチャを繰り返すようにする
	void SetTextureRepeat(bool _enable, float unitSize = 32.0f);

	// 色を変える
	void SetColor(const DirectX::SimpleMath::Vector4& _color);

	// コンポーネントのIDを返す
	TypeID GetComponentType(void) const override { return TypeID::SpriteComponent; }

private:
	class TransformComponent* owner_transform_;	// 位置情報

	std::shared_ptr<Texture> texture_;	// テクスチャ

	int current_frame_x_;	// 現在のフレームX
	int current_frame_y_;	// 現在のフレームY
};
#endif	// SPRITE_COMPONENT_H_