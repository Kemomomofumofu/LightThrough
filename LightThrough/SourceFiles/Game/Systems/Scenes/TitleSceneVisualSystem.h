#pragma once
/**
 * @file TitleSceneVisualSystem.h
 * @brief タイトルシーンのビジュアル更新システム
 */

 // ---------- インクルード ---------- // 
#include <Game/ECS/ISystem.h>

namespace ecs {
	/**
	 * @brief タイトルメニューの見た目を更新するシステム
	 * @details TitleSceneItemとSpriteRendererを持つEntityを対象とし、
	 *          選択状態に応じてスプライトの色を変更する。
	 */
	class TitleSceneVisualSystem : public ISystem {
	public:
		explicit TitleSceneVisualSystem(const SystemDesc& _desc);

		virtual void Init() override;
		virtual void Update(float _dt) override;

	private:
		float pulse_time_ = 0.0f;	// 選択中ボタンの点滅用タイマー
	};
}