//==================================================
// [BoxColliderComponent.h] 四角の当たり判定コンポーネント
// 著者：有馬啓太
//--------------------------------------------------
// 説明：四角の当たり判定のコンポーネント
//==================================================
#ifndef BOX_COLLIDER_COMPONENT_H_
#define BOX_COLLIDER_COMPONENT_H_
/*----- インクルード -----*/
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <SimpleMath.h>
#include "GameObjects/IComponent.h"


using namespace DirectX::SimpleMath;

/*----- 構造体宣言 -----*/
struct AABB {
	Vector2 min;		// 矩形の左下
	Vector2 max;		// 矩形の右上

	Vector2 Center() const {
		return(min + max) * 0.5f;
	}
};

/*----- 前方宣言 -----*/
class TransformComponent;

//--------------------------------------------------
// 四角形の当たり判定
//--------------------------------------------------
class BoxColliderComponent :public IComponent
{
public:
	BoxColliderComponent(GameObject* _owner, int _updateOrder = 60);
	~BoxColliderComponent();

	void Init(void) override;
	void Uninit(void) override;
	void Update(void) override;


	bool CheckCollisionCollider(BoxColliderComponent* _other);

	TypeID GetComponentType(void) const override { return TypeID::BoxColliderComponent; }
 

	void SetSize(const Vector2 _size) { size_ = _size; }
	auto GetSize(void) const { return size_; }

	void SetAABB(const AABB& _hitbox) { hit_box_ = _hitbox; }

	// 当たり判定の位置を決定したり
	void SetWorldHitBox(Vector3 _position);
	void SetWorldHitBox(const AABB& _worldHitbox) { hit_box_ = _worldHitbox; }
	auto GetWorldHitBox(void) const { return hit_box_; }

	// 押し出し処理
	void ResolveCollision(BoxColliderComponent* _other);

	// トリガーかどうか
	void SetIsTrigger(bool _isTrigger) { is_trigger_ = _isTrigger; }
	auto IsTrigger(void) const { return is_trigger_; }

	// 衝突しているオブジェクトリスト
	void SetPreviousCollisions(const std::unordered_set<BoxColliderComponent*>& _collisions) { previous_collisions_ = _collisions; }
	auto GetPreviousCollisions(void) const { return previous_collisions_; }

	// イベント
	std::function<void(BoxColliderComponent* _other)> OnCollisionEnter;
	std::function<void(BoxColliderComponent* _other)> OnCollisionStay;
	std::function<void(BoxColliderComponent* _other)> OnCollisionExit;
	std::function<void(BoxColliderComponent* _other)> OnTriggerEnter;
	std::function<void(BoxColliderComponent* _other)> OnTriggerStay;
	std::function<void(BoxColliderComponent* _other)> OnTriggerExit;


private:
	class TransformComponent* owner_transform_{};

private:
	// 前フレームからの衝突対象リスト
	std::unordered_set<BoxColliderComponent*> previous_collisions_;


	Vector2 size_{};	// 直径
	AABB hit_box_{};	// 当たり判定の矩形

	// 衝突解決しないならtrue
	bool is_trigger_{};
};

#endif // BOX_COLLIDER_COMPONENT_H_