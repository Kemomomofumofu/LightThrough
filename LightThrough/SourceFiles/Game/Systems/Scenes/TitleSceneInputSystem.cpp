/**
 * @file TitleSceneInputSystem.cpp
 * @brief タイトルシーンの入力処理システム
 */

 // ---------- インクルード ---------- // 
#include <Game/Systems/Scenes/TitleSceneInputSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Scene/SceneManager.h>
#include <Game/InputSystem/InputSystem.h>

#include <Game/Components/Scenes/TitleSceneState.h>
#include <Game/Components/Scenes/TitleSceneItem.h>

#include <Debug/Debug.h>

namespace ecs {

	TitleSceneInputSystem::TitleSceneInputSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
	}

	void TitleSceneInputSystem::Init()
	{
		Signature sig;
		sig.set(ecs_.GetComponentType<TitleSceneState>());
		ecs_.SetSystemSignature<TitleSceneInputSystem>(sig);
	}

	void TitleSceneInputSystem::Update(float _dt)
	{
		// タイトルシーンでなければスキップ
		if (scene_manager_.GetActiveScene() != "TitleScene") { return; }

		auto& input = input::InputSystem::Get();

		for (auto& e : entities_) {
			auto* state = ecs_.GetComponent<TitleSceneState>(e);
			if (!state) { continue; }

			// ボタン総数を毎フレーム計上する（動的にItemが増減しても対応）
			int count = 0;
			// 全TitleSceneItemを走査してitemCountを更新
			// SetSystemSignatureはStateに紐づいているのでItemのEntityは別途走査
			ecs_.ForEachComponent<TitleSceneItem>(
				[&count](Entity, const TitleSceneItem&) {
					++count;
				}
			);
			state->itemCount = count;

			if (state->itemCount <= 0) { continue; }

			// 上下キーで選択変更
			if (input.IsKeyTrigger(VK_UP) || input.IsKeyTrigger('W')) {
				state->selectedIndex = (state->selectedIndex - 1 + state->itemCount) % state->itemCount;
				state->confirmed = false;
			}
			if (input.IsKeyTrigger(VK_DOWN) || input.IsKeyTrigger('S')) {
				state->selectedIndex = (state->selectedIndex + 1) % state->itemCount;
				state->confirmed = false;
			}

			// 決定キー
			if (input.IsKeyTrigger(VK_RETURN) || input.IsKeyTrigger(VK_SPACE)) {
				state->confirmed = true;

				// 選択中のItemを探してアクションを実行
				ecs_.ForEachComponent<TitleSceneItem>(
					[this, &state](Entity, const TitleSceneItem& _item) {
						if (_item.index == state->selectedIndex) {
							ExecuteAction(_item.action);
						}
					}
				);
			}
		}
	}

	void TitleSceneInputSystem::ExecuteAction(int _action)
	{
		const auto action = static_cast<TitleAction>(_action);
		switch (action) {
		case TitleAction::StartGame:
			DebugLogInfo("[TitleScene] StartGame アクション実行");
			scene_manager_.ChangeScene("Stage_1");
			break;
		case TitleAction::ExitGame:
			DebugLogInfo("[TitleScene] ExitGame アクション実行");
			PostQuitMessage(0);
			break;
		default:
			DebugLogWarning("[TitleScene] 不明なアクション: {}", _action);
			break;
		}
	}
}