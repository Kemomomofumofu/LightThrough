/**
 * @file MoveDirectionResolveSystem.cpp
 */


 // // ---------- インクルード ---------- //
#include <Game/Systems/Initialization/Resolve/MoveDirectionSourceResolveSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Components/MoveDirectionSource.h>
#include <Game/Components/Object/Name.h>


namespace ecs {
	// ! @brief 初期化
	void MoveDirectionSourceResolveSystem::Init()
	{
		Signature signature;
		signature.set(ecs_.GetComponentType<MoveDirectionSource>());
		ecs_.SetSystemSignature<MoveDirectionSourceResolveSystem>(signature);
	}

	// ! @brief 更新
	void MoveDirectionSourceResolveSystem::Update(float _dt)
	{
		//if (!active_) { return; }

		for (auto const& e : entities_) {
			auto& moveDirSource = ecs_.GetComponent<MoveDirectionSource>(e);
			// targetNameからtargetを解決
			auto entitiesWithName = ecs_.GetEntitiesWithComponent<Name>();
			for (auto const& targetEntity : entitiesWithName) {
				auto& nameComp = ecs_.GetComponent<ecs::Name>(targetEntity);
				if (nameComp.value == moveDirSource.targetName) {
					moveDirSource.target = targetEntity;
					break;
				}
			}
		}

		// memo: 一度だけ実行なので非アクティブ化、今後自動化する
		active_ = false;
	}
}