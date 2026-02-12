#pragma once
/**
 * @file LightPlaceRequest.h
 * @brief ライト設置リクエストコンポーネント
 */

 // ---------- インクルード ---------- //
#include <DirectXMath.h>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs
{
	/**
	 * @brief ライト設置リクエストコンポーネント
	 */
	struct LightPlaceRequest {
		// int lightType{ 0 };			// ライトタイプ memo: 今後ポイントライトなどが追加した時用
		DirectX::XMFLOAT3 spawnPos{};		// 生成位置
		DirectX::XMFLOAT3 spawnDir{};		// 生成方向
	};
} // namespace ecs

