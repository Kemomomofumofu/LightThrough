#pragma once
/**
 * @file TitleSceneState.h
 * @brief タイトルシーンの状態管理コンポーネント
 */

 // ---------- インクルード ---------- // 
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {
	/**
	 * @brief タイトルメニューの状態を保持するコンポーネント
	 * @details シーン内に1つだけ存在するManagerEntityに付与する。
	 */
	struct TitleSceneState {
		int selectedIndex = 0;	// 現在選択中のインデックス
		int itemCount = 0;		// ボタンの総数（Systemが自動計上）
		bool confirmed = false;	// 決定が押されたか
	};
}

// selectedIndex, itemCount: 表示のみ（ランタイム管理値）
ECS_REFLECT_BEGIN(ecs::TitleSceneState)
ECS_REFLECT_FIELD_INSPECT_ONLY(selectedIndex),
ECS_REFLECT_FIELD_INSPECT_ONLY(itemCount)
ECS_REFLECT_END()
