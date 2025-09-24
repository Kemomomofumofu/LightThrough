/**
 * @file Camera.h
 * @brief カメラコンポーネント
 * @author Arima Keita
 * @date 2025-09-04
 */

 // ---------- インクルード ---------- //
#include <DirectXMath.h>


namespace ecs {

	/**
	 * @brief カメラの情報
	 *
	 * 
	 */
	struct Camera {
		float fovY = 100.0f; // 垂直視野角(degree)
		float aspectRatio = 16.0f / 9.0f; // アスペクト比
		float nearZ = 0.1f; // ニアクリップ面
		float farZ = 100.0f; // ファークリップ面

		bool isMain = false;	// メインカメラフラグ [ToDo] (仮置き)
		bool isActive = true;	// アクティブフラグ [ToDo] (仮置き)

		DirectX::XMMATRIX view{};	// ビュー行列
		DirectX::XMMATRIX proj{};	// プロジェクション行列
	};
}