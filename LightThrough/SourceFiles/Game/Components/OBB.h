#pragma once
/**
 * @file OBB.h
 * @brief OBBに必要な情報
 * @author Arima Keita
 * @date 2025-10-03
 */



// ---------- インクルード ---------- // 
#include <cstdint>
#include <DirectXMath.h>

namespace ecs {
	/**
	 * @brief OBBコンポーネント
	*/
	struct OBB {
		DirectX::XMFLOAT3 localCenter{};
		DirectX::XMFLOAT3 halfExtents{ 0.5f, 0.5f, 0.5f };
		uint32_t layer = 0x00000001; // レイヤー
		uint32_t mask = 0xFFFFFFFF;  // マスク
		bool isTrigger = false;		// 当たり判定を持つか
		bool isStatic = false;		// 静的か
	};
}