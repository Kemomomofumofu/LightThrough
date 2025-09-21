#pragma once
/**
 * @file IScene.h
 * @brief シーンのインターフェース
 * @author Arima Keita
 * @date 2025-09-22
 */

 // ---------- インクルード ---------- //
#include <string>

namespace scene {
	/**
	 * @brief シーンの規定クラス
	 * 
	 * シーンの基本的なインターフェースを提供するクラス
	 * 
	 */
	class IScene {
	public:
		virtual ~IScene() = default;

		virtual void OnEnter() {};	// 入るときの処理
		virtual void OnUpdate(float dt) = 0; // 更新
		virtual void OnExit() {};	// 出るときの処理

		virtual const std::string& GetName() const = 0;
	};
}
