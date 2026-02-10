#pragma once
/**
 * @file TextureHandleResolveSystem.h.h
 */


 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>
#include <DX3D/Graphics/Textures/TextureRegistry.h>


namespace ecs {
	/**
	 * @brief Textureの関連付けを解決するシステム
	 */
	class TextureHandleResolveSystem : public ISystem {
	public:
		//! @brief コンストラクタ
		explicit TextureHandleResolveSystem(const SystemDesc& _desc)
			: ISystem(_desc)
			, texture_registry_(_desc.textureRegistry)
		{
		}
		//! @brief 初期化
		void Init() override;
		//! @brief 更新
		void Update(float _dt) override;
	private:
		dx3d::TextureRegistry& texture_registry_;
	};
}