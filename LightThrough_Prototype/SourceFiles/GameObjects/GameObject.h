//==================================================
// [GameObject.h] ゲームオブジェクトベースヘッダ
// 著者：有馬啓太
//--------------------------------------------------
// 説明：ゲームオブジェクトの継承元
//==================================================
#ifndef GAMEOBJECT_H_
#define GAMEOBJECT_H_


/*----- インクルード -----*/
#include <memory>
#include <vector>
#include <iostream>
#include <format>
#include <typeinfo>

#include "../InputManager.h"
#include "IComponent.h"
#include "Component/TransformComponent.h"
#include "Component/CameraComponent.h"
#include "Component/RenderComponent/SpriteComponent.h"

/*----- 構造体定義 -----*/

/*----- 前方宣言 -----*/
class GameManager;

//--------------------------------------------------
// ゲームオブジェクトクラス
//--------------------------------------------------
class GameObject
{
public:
	enum class TypeID
	{
		None = -1
		// 自分自身
		, GameObject

		// アクター（独立した役割を持つゲームオブジェクト）
		// ここに追加したゲームオブジェクトを追加していく
		, Camera
		, Player
		, Block

		// ゲームオブジェクトのIDの最大値
		, MAX
	};

	// ゲームオブジェクトが所有する方のID
	static const char* GameObjectTypeNames[static_cast<int>(TypeID::MAX)];

	// オブジェクトの状態
	enum class State
	{
		None = -1
		, Active		// Updateされる
		, Paused		// Updateされない
		, Dead			// GameObjectsリストから除外される(削除はされない)
		, ColliderOut	// ColliderManagerから削除される
		, MAX			// 状態の最大値
	};

public:
	GameObject(GameManager* _gameManager, std::string _objectName = "不明なオブジェクト");
	virtual ~GameObject(void);

	void Init(void);
	void Uninit(void);
	void Update(void);

	void InitComponent(void);
	void UpdateComponents(void);

	virtual void InitGameObject(void) {};	// オーバーライド用
	virtual void UpdateGameObject(void) {};	// オーバーライド用


	// ゲームオブジェクトの名前設定、取得
	void SetObjectName(std::string _objectName) { object_name_ = _objectName; }
	auto& GetObjectName() const { return object_name_; }

	// コンポーネントの追加、削除
	void AddComponent(IComponent* component);
	void RemoveComponent(IComponent* component);

	//--------------------------------------------------
	// @param	取得したい component(T)
	// @brief	GameObjectの  components_からにキャストする
	// @return	見つかれば targetを	見つからなければ nullptr を返す
	//--------------------------------------------------
	template <typename T>
	inline T* GetComponent() const {

		T* target = nullptr;
		for (auto component : components_)
		{
			target = dynamic_cast<T*>(component);
			if (target)
			{
				return target;
			}
		}
		return nullptr;
	}

	// トランスフォームを取得
	TransformComponent* GetTransformComponent() const { return transform_component_; }

	// コンポーネントリストの取得
	const auto& GetComponents() const { return components_; }

	// ゲームオブジェクトの状態の変更
	void SetState(State _state) { state_ = _state; }
	State& GetState(void) { return state_; }

	virtual TypeID GetType(void) { return TypeID::GameObject; }

	auto& GetGameManager(void) { return game_manager_; }

protected:
	// GameObjectの所有者
	GameManager* game_manager_{};

	// オブジェクトの名前
	std::string object_name_{};

	// GameObjectの状態
	State state_{};

	// 所有コンポーネント
	std::vector<IComponent*> components_{};

	// 姿勢制御コンポーネント
	TransformComponent* transform_component_{};

};

#endif	// GAMEOBJECT_H_


