#pragma once
/**
 * @file GameObjectType.h
 * @brief ゲームオブジェクトのenum
 * @author Arima Keita
 * @date 2025-08-20
 */

namespace LightThrough {
	enum class GameobjectType {
		None = 0,	// 何もない
		Camera,		// カメラ
		Player,		// プレイヤー
		UI,			// UIオブジェクト
		Max			// 最大値
	};
}