#pragma once
/**
 * @file InstanceData.h
 * @brief インスタンスデータ構造体
 * @author Arima Keita
 * @date 2025-09-27
 */


 // ---------- インクルード ---------- //
#include <DirectXMath.h>

namespace dx3d {
	/**
	 * @brief インスタンスデータ構造体
	 *
	 * インスタンスレンダリング用のデータ構造体
	 */
	struct InstanceDataMain {
		DirectX::XMFLOAT4X4 world;	// ワールド行列
		DirectX::XMFLOAT4 color{1, 1, 1, 1};	// インスタンスカラー
	};

	//! @brief シャドウマップ用インスタンスデータ
	struct InstanceDataShadow {
		DirectX::XMFLOAT4X4 world;	// ワールド行列
	};

	struct InstanceDataSprite {
		DirectX::XMFLOAT4X4 world;	// ワールド行列
		DirectX::XMFLOAT4 color{1, 1, 1, 1};	// インスタンスカラー
	};
}