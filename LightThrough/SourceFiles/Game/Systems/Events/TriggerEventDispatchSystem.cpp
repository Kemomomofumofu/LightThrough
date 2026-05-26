/**
 * @file TriggerEventDispatchSystem.cpp
 */

 // ---------- インクルード ---------- //
#include <Game/Systems/Events/TriggerEventDispatchSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Scene/SceneManager.h>

#include <Game/Components/Core/Transform.h>
#include <Game/Components/Events/TriggerEvents.h>

#include <Debug/Debug.h>

namespace ecs {
	TriggerEventDispatchSystem::TriggerEventDispatchSystem(const SystemDesc& _desc)
		: ISystem(_desc) {
	}
	void TriggerEventDispatchSystem::Init()
	{
		Signature sig;
		sig.set(ecs_.GetComponentType<TriggerContact>());
		ecs_.SetSystemSignature<TriggerEventDispatchSystem>(sig);
	}

	//! @brief トリガーイベントの処理
	void TriggerEventDispatchSystem::FixedUpdate(float _dt)
	{
		// 範囲外参照を避けるため処理対象をコピー
		std::vector<Entity> ents(entities_.begin(), entities_.end());

		for (const Entity e : ents) {
			if (!ecs_.HasComponent<TriggerContact>(e)) { continue; }
			auto* tc = ecs_.GetComponent<TriggerContact>(e);

			for (const auto& entry : tc->entries) {
				switch (entry.type) {
				// ---------- ゴール ---------- // 
				case TriggerType::Goal:
				{
					if (entry.param.empty()) {
						DebugLogWarning("[TriggerEvents] Goalのparam(遷移先シーン名)が空\n");
						break;
					}
					DebugLogInfo("[TriggerEvents] Goal -> {}\n", entry.param);

					// シーン遷移情報の作成
					// memo: ステージのゴール地点に次ステージの開始地点を配置するための情報
					scene::SceneTransitionInfo info{};
					info.goalPosition = ecs_.GetComponent<Transform>(entry.other)->GetWorldPosition();
					info.startPointName = "StartLight";	// todo: 指定できるか、タグで探すのもいいかも

					scene_manager_.RequestChangeScene(entry.param, info);
					break;
				}
				// ---------- つかむ ---------- //
				case TriggerType::Grab:
				{
					if (!ecs_.HasComponent<GrabRequest>(e)) {
						GrabRequest req{};
						req.grabber = e;
						req.target = entry.other;
						ecs_.RequestAddComponent<GrabRequest>(e, req);
					}
					DebugLogInfo("[TriggerEvents] Grab: {} -> {}\n", e.id_, entry.other.id_);
					break;
				}

				default:
					break;
				}
			}

			// 処理後はイベントを削除
			ecs_.RequestRemoveComponent<TriggerContact>(e);
		}

	}
} // namespace ecs