#pragma once
/**
 * @file TriggerEvents.h
 * @brief トリガーイベント用コンポーネント
 */

 // ---------- インクルード ---------- // 
#include <vector>
#include <string>
#include <Game/ECS/Entity.h>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs
{
	/**
	 * @brief トリガーの種類
	 */
	enum class TriggerType : uint8_t {
		None = 0,
		Goal,	// ゴール判定
		Grab,	// オブジェクトをつかむ
	};

	/**
	 * @brief トリガーの定義
	 * @details Collider::isTrigger=true のEntity に付与する。
	 */
	struct TriggerTag
	{
		int type = 0;	// トリガーの種類
		std::string param{};	// 汎用パラメータ	(遷移先シーン名など)
	};

	/**
	 * @brief 1フレーム中に発生したトリガー衝突イベント
	 * @details 衝突をシステムで検出して、対象Entityに付与する。
	 */
	struct TriggerContact
	{
		struct Entry {
			Entity other;		// 衝突相手
			TriggerType type;	// 相手のTriggerType
			std::string param;	// 相手のparam
		};
		std::vector<Entry> entries{};
	};

	/**
	 * @brief つかみリクエスト
	 */
	struct GrabRequest
	{
		Entity grabber;	// つかむ側
		Entity target;	// つかまれる側
	};
} // namespace ecs

ECS_REFLECT_BEGIN(ecs::TriggerTag)
ECS_REFLECT_FIELD(type),
ECS_REFLECT_FIELD(param)
ECS_REFLECT_END()