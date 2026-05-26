/**
 * @file TitleSceneVisualSystem.cpp
 * @brief タイトルシーンのビジュアル更新システム
 */

 // ---------- インクルード ---------- // 
#include <Game/Systems/Scenes/TitleSceneVisualSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Scene/SceneManager.h>

#include <Game/Components/Scenes/TitleSceneState.h>
#include <Game/Components/Scenes/TitleSceneItem.h>
#include <Game/Components/Render/SpriteRenderer.h>

#include <cmath>

namespace ecs {

	TitleSceneVisualSystem::TitleSceneVisualSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
	}

	void TitleSceneVisualSystem::Init()
	{
		Signature sig;
		sig.set(ecs_.GetComponentType<TitleSceneItem>());
		sig.set(ecs_.GetComponentType<SpriteRenderer>());
		ecs_.SetSystemSignature<TitleSceneVisualSystem>(sig);
	}

	void TitleSceneVisualSystem::Update(float _dt)
	{
		// タイトルシーンでなければスキップ
		if (scene_manager_.GetActiveScene() != "TitleScene") { return; }

		pulse_time_ += _dt;

		// TitleSceneStateから現在の選択インデックスを取得
		int selectedIndex = 0;
		ecs_.ForEachComponent<TitleSceneState>(
			[&selectedIndex](Entity, const TitleSceneState& _state) {
				selectedIndex = _state.selectedIndex;
			}
		);

		// 選択中アイテムの色/スケール演出
		const float pulse = 0.5f + 0.5f * std::sinf(pulse_time_ * 4.0f); // 0..1 の脈動

		for (auto& e : entities_) {
			auto* item = ecs_.GetComponent<TitleSceneItem>(e);
			auto* sprite = ecs_.GetComponent<SpriteRenderer>(e);
			if (!item || !sprite) { continue; }

			if (item->index == selectedIndex) {
				// 選択中: 明るい色で点滅
				const float brightness = 0.8f + 0.2f * pulse;
				sprite->color = { brightness, brightness, brightness, 1.0f };
			}
			else {
				// 非選択: 暗めの色
				sprite->color = { 0.4f, 0.4f, 0.4f, 1.0f };
			}
		}
	}
}