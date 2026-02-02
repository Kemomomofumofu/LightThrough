/**
 * @file LightReferenceResolveSystem.cpp
 */


 // // ---------- インクルード ---------- //
#include <Game/Systems/Initialization/Resolve/LightReferenceResolveSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Input/PlayerController.h>
#include <Game/Components/Core/Name.h>


namespace ecs {
	// ! @brief 初期化
	void LightReferenceResolveSystem::Init()
	{
		Signature signature;
		signature.set(ecs_.GetComponentType<PlayerController>());
		ecs_.SetSystemSignature<LightReferenceResolveSystem>(signature);
	}

	// ! @brief 更新
	void LightReferenceResolveSystem::Update(float _dt)
	{
		//if (!active_) { return; }

		for (auto const& e : entities_) {
			auto pc = ecs_.GetComponent<PlayerController>(e);
			// targetNameからtargetを解決
			auto entitiesWithName = ecs_.GetEntitiesWithComponent<Name>();
			for (auto const& targetEntity : entitiesWithName) {
				auto nameComp = ecs_.GetComponent<ecs::Name>(targetEntity);
				if (nameComp->value == pc->handLightName) {
					pc->handLight = targetEntity;
					break;
				}
			}
		}

		// memo: 一度だけ実行なので非アクティブ化、今後自動化する
		active_ = false;
	}
}