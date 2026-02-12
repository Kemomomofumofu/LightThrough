#pragma once
/**
 * @file Camera.h
 * @brief カメラコンポーネント
 * @author Arima Keita
 * @date 2025-09-04
 */

 // ---------- インクルード ---------- //
#include <DirectXMath.h>
#include <Game/Serialization/ComponentReflection.h>

namespace ecs {

	/**
	 * @brief カメラの情報
	 *
	 * 
	 */
	struct Camera {
		float fovY = 100.0f; // 垂直視野角(degree)
		float aspectRatio = 16.0f / 9.0f; // アスペクト比
		float nearZ = 0.05f; // ニアクリップ面
		float farZ = 100.0f; // ファークリップ面

		bool isMain = false;	// メインカメラフラグ [ToDo] (仮置き)
		bool isActive = true;	// アクティブフラグ [ToDo] (仮置き)

		DirectX::XMFLOAT4X4 view{};	// ビュー行列
		DirectX::XMFLOAT4X4 proj{};	// プロジェクション行列
	};
}


ECS_REFLECT_BEGIN(ecs::Camera)
ECS_REFLECT_FIELD(fovY),
ECS_REFLECT_FIELD(aspectRatio),
ECS_REFLECT_FIELD(nearZ),
ECS_REFLECT_FIELD(farZ),
ECS_REFLECT_FIELD(isMain),
ECS_REFLECT_FIELD(isActive)
ECS_REFLECT_END()
