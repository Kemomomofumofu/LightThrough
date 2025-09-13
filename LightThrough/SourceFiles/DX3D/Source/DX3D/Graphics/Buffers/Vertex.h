#pragma once
/**
 * @file Vertex.h
 * @brief 頂点データ構造
 * @author Arima Keita
 * @date 2025-09-12
 */

// ---------- インクルード ---------- // 
#include <DirectXMath.h>

namespace dx3d {
	/**
	 * @brief 頂点データ構造
	 *
	 * 頂点の位置と色、を保持する構造体
	 */
	struct Vertex {
		DirectX::XMFLOAT3 position; // 頂点の位置
		DirectX::XMFLOAT4 color;    // 頂点の色
		DirectX::XMFLOAT2 uv;       // 頂点のUV座標
		DirectX::XMFLOAT3 normal;   // 頂点の法線ベクトル
	};
}