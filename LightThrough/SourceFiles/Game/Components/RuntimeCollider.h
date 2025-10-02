#pragma once
/**
 * @file RuntimeCollider.h
 * @brief コライダーシステムの内部キャッシュ用
 * @author Arima Keita
 * @date 2025-09-15
 */



// ---------- インクルード ---------- // 
#include <cstdint>
#include <DirectXMath.h>

namespace ecs {
	/**
	 * @brief ランタイム用OBBコンポーネント
	*/
	struct RuntimeCollider {
		DirectX::XMFLOAT3 worldCenter{};
		DirectX::XMFLOAT3 axis[3]{ {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
		DirectX::XMFLOAT3 halfWorld{};
		DirectX::XMFLOAT3 aabbMin{};
		DirectX::XMFLOAT3 aabbMax{};
		uint64_t lastSyncFrame = 0;
	};
}