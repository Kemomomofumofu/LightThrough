/**
 * @file ObjectResolveSystem.cpp
 */


// ---------- インクルード ---------- // 
#include <unordered_map>
#include <string>

#include <Game/Systems/Initialization/Resolve/ObjectResolveSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Object/ObjectRoot.h>
#include <Game/Components/Object/ObjectChild.h>
#include <Game/Components/Object/Name.h>
#include <Game/Components/Transform.h>

#include <Game/ECS/ECSUtils.h>
#include <Debug/Debug.h>

namespace ecs {

	//! @brief 初期化
	void ObjectResolveSystem::Init()
	{
		Signature signature;
		signature.set(ecs_.GetComponentType<ObjectChild>());
		ecs_.SetSystemSignature<ObjectResolveSystem>(signature);
	}

	//! @brief 実行
	void ObjectResolveSystem::Update(float _dt)
	{
		//if (!active_) { return; }

		// 対象がいなければ終了
		if(entities_.empty()) {
			active_ = false;
			return;
		}

		std::unordered_map<std::string, Entity> rootMap;

		// ルートオブジェクトのマップを作成
		for (auto e : ecs_.GetEntitiesWithComponent<ObjectRoot>()) {
			const auto& r = ecs_.GetComponent<ObjectRoot>(e);
			rootMap.emplace(r.name, e);
		}

		// 一応名前付きオブジェクトも登録
		for (auto e : ecs_.GetEntitiesWithComponent<Name>()) {
			if (rootMap.find(ecs_.GetComponent<Name>(e).value) == rootMap.end()) {
				rootMap.emplace(ecs_.GetComponent<Name>(e).value, e);
			}
		}

		// 子オブジェクトの親を解決
		bool anyUnresolved = false;
		for (auto e : entities_) {
			auto& child = ecs_.GetComponent<ObjectChild>(e);
			if (child.root.IsInitialized()) { continue; } // すでに親がセット済みならスキップ

			// 親の名前が空なら解決できないのでスキップ
			if (child.rootName.empty()) {
				anyUnresolved = true;
				continue;
			}

			// マップから親を探す
			auto it = rootMap.find(child.rootName);
			if (it != rootMap.end()) {
				child.root = it->second;
				GameLogFInfo("Root: {}, Child: {}", child.rootName, ecs_.GetComponent<Name>(e).value);
			}
			else {
				GameLogFWarning("[ObjectResolveSystem] 親オブジェクト '{}' が見つかりません。", child.rootName);
				anyUnresolved = true;
			}

		}
			// すべて解決出来たら非アクティブ化
			if (!anyUnresolved) {
				active_ = false;
			}
	}
}