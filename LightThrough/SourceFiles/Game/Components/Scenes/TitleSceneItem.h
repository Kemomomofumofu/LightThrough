#pragma once
/**
 * @file TitleSceneItem.h
 * @brief タイトルシーンのボタン項目コンポーネント
 */

 // ---------- インクルード ---------- // 
#include <string>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {
	/**
	 * @brief ボタンのアクション種別
	 */
	enum class TitleAction : int {
		None = -1,
		StartGame,	// ゲーム開始
		ExitGame,	// ゲーム終了
	};

	/**
	 * @brief タイトルシーンのメニュー項目コンポーネント
	 * @details 各ボタンEntityに付与する。
	 */
	struct TitleSceneItem {
		int index = 0;				// メニュー内でのインデックス（0始まり）
		int action = 0;			// 実行するアクション（TitleAction）
	};
}

ECS_REFLECT_BEGIN(ecs::TitleSceneItem)
ECS_REFLECT_FIELD(index),
ECS_REFLECT_FIELD(action)
ECS_REFLECT_END()
