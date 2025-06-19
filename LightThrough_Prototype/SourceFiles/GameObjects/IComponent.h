//==================================================
// [IComponent.h] コンポーネントベースヘッダ
// 著者：有馬啓太
//--------------------------------------------------
// 説明：コンポーネントの大元
//==================================================
#ifndef COMPONENT_H_
#define COMPONENT_H_

/*----- インクルード -----*/
#include <memory>

/*----- 構造体定義 -----*/

/*----- 前方宣言 -----*/
class GameObject;

//--------------------------------------------------
// コンポーネントクラス
//--------------------------------------------------
class IComponent 
{
public:
	enum class TypeID
	{
		None = -1
		// ベースクラス
		, IComponent

		// ここに追加したコンポーネントを書いていく
		// カメラ
		, CameraComponent

		// レンダリング
		, RenderComponent
		, SpriteComponent

		// 基本機能
		, TransformComponent	// 姿勢制御

		, BoxColliderComponent


		// コンポーネントの最大値
		, MAX
	};

	static const char* ComponentTypeNames[static_cast<int>(TypeID::MAX)];

public:
	// updateOrderは0以上、100以下で設定
	IComponent(GameObject* _owner, int updateOrder = 50);
	virtual ~IComponent();

	virtual void Init(void) = 0;
	virtual void Uninit(void) = 0;
	virtual void Update(void) = 0;

    auto GetOwner(void) { return owner_; }
	int GetUpdateOrder(void) const { return update_order_; }

	virtual TypeID GetComponentType() const = 0;



protected:
	GameObject* owner_;	// 自分（コンポーネント）の所有者
	int update_order_{};			// 自分自身の更新順位
};


#endif	// COMPONENT_H_