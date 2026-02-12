/**
 * @file MeshHandleResolveSystem.cpp
 */


 // // ---------- インクルード ---------- //
#include <Game/Systems/Initialization/Resolve/MeshHandleResolveSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Core/Name.h>
#include <Game/Components/Render/MeshRenderer.h>


namespace ecs {
	// ! @brief 初期化
	void MeshHandleResolveSystem::Init()
	{
		Signature signature;
		signature.set(ecs_.GetComponentType<MeshRenderer>());
		ecs_.SetSystemSignature<MeshHandleResolveSystem>(signature);
	}

	// ! @brief 更新
	void MeshHandleResolveSystem::Update(float _dt)
	{
		//if (!active_) { return; }

		for (auto const& e : entities_) {
			auto mr = ecs_.GetComponent<MeshRenderer>(e);
			// meshNameからmeshHandleを解決
			if (mr->handle.IsValid()) { continue; } // すでに解決済み
			auto handle = mesh_registry_.GetHandleByName(mr->meshName);
			if (handle.IsValid()) {
				mr->handle = handle;
			}
			else {
				DebugLogWarning("[MeshHandleResolveSystem] 未登録のMesh: {}", mr->meshName);
			}
		}

		// memo: 一度だけ実行なので非アクティブ化、今後自動化する
		active_ = false;
	}
}