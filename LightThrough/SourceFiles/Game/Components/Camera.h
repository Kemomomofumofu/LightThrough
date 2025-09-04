/**
 * @file Camera.h
 * @brief カメラコンポーネント
 * @author Arima Keita
 * @date 2025-09-04
 */


namespace ecs {
	struct Camera {
		float fovY = 45.0f; // 垂直視野角
		float aspectRatio = 16.0f / 9.0f; // アスペクト比
		float nearZ = 0.1f; // ニアクリップ面
		float farZ = 100.0f; // ファークリップ面
		bool isActive = false; // アクティブフラグ
	};
}