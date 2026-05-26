#pragma once
/**
 * @file TitleSceneInputSystem.h
 * @brief タイトルシーンの入力処理システム
 */

 // ---------- インクルード ---------- // 
#include <Game/ECS/ISystem.h>

namespace ecs {
	/**
	 * @brief タイトルメニューの入力を処理するシステム
	 * @details TitleSceneStateを持つEntityを対象とし、
	 *          上下キーで選択変更、決定キーでアクション実行を行う。
	 */
	class TitleSceneInputSystem : public ISystem {
	public:
		explicit TitleSceneInputSystem(const SystemDesc& _desc);

		virtual void Init() override;
		virtual void Update(float _dt) override;

	private:
		/**
		 * @brief 選択されたアクションを実行する
		 * @param _action アクション種別
		 */
		void ExecuteAction(int _action);
	};
}
