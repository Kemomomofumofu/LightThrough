/**
 * @file TextureHandleResolveSystem.cpp
 */


 // // ---------- インクルード ---------- //
#include <Game/Systems/Initialization/Resolve/TextureHandleResolveSystem.h>
#include <Game/ECS/Coordinator.h>

#include <Game/Components/Core/Name.h>
#include <Game/Components/Render/SpriteRenderer.h>


namespace ecs {
	// ! @brief 初期化
	void TextureHandleResolveSystem::Init()
	{
		Signature signature;
		signature.set(ecs_.GetComponentType<SpriteRenderer>());
		ecs_.SetSystemSignature<TextureHandleResolveSystem>(signature);
	}

	// ! @brief 更新
	void TextureHandleResolveSystem::Update(float _dt)
	{
		//if (!active_) { return; }

		for (auto const& e : entities_) {
			auto sr = ecs_.GetComponent<SpriteRenderer>(e);
			// textureNameからtextureHandleを解決
			if (sr->handle.IsValid()) { continue; } // すでに解決済み
			auto handle = texture_registry_.Load(sr->spriteName);
			if (handle.IsValid()) {
				sr->handle = handle;
			}
			else {
				DebugLogWarning("[TextureHandleResolveSystem] 存在しないTexture: {}", sr->spriteName);
			}
		}

		// memo: 一度だけ実行なので非アクティブ化、今後自動化する
		active_ = false;
	}
}