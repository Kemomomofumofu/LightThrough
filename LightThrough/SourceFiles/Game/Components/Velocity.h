#pragma once
/**
 * @file Velocity.h
 * @brief 速度コンポーネント
 * @author Arima Keita
 * @date 2025-08-17
 */

 // ---------- インクルード ---------- //
#include <DirectXMath.h>


namespace ecs {
	/**
	 * @brief 速度コンポーネント
	 *
	 * 物体の速度を表すコンポーネント。
	 * 位置の更新に使用。
	 */
	struct Velocity {
		DirectX::XMFLOAT3 velocity = { 0.0f, 0.0f, 0.0f };	// 速度ベクトル
	};
}
