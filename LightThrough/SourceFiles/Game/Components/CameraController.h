#pragma once
/**
 * @file CameraController.h
 * @brief カメラ制御コンポーネント
 * @author Arima Keita
 * @date 2025-09-05
 */
 // ---------- インクルード ---------- //
#include <DirectXMath.h>

namespace ecs {
	enum class CameraMode {
		FPS,	// 一人称視点
		Orbit,	// オービット
	};;


	/**
	 * @brief カメラコントローラー
	 * @param mode カメラモード
	 * @param moveSpeed 移動速度
	 * @param mouseSensitivity マウス感度
	 *
	 * カメラを操作に関する情報を持つComponent
	 */
	struct CameraController {
		CameraMode mode = CameraMode::FPS;	// カメラモード

		float moveSpeed = 5.0f;				// 移動速度
		float mouseSensitivity = 1.0f;		// マウス感度

		// FPS用
		float yaw = 0.0f;	// Y軸回転
		float pitch = 0.0f;	// X軸回転

		DirectX::XMFLOAT3 orbitTarget{ 0.0f, 0.0f, 0.0f };	// 注視点
		float orbitDistance = 5.0f;							// 注視点からの距離
		float orbitYaw = 0.0f;								// Y軸回転
		float orbitPitch = 0.3f;							// X軸回転

		// オプション
		bool invertY = false;	// Y軸反転
	};
}
