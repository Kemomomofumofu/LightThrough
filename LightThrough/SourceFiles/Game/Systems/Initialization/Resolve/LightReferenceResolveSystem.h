#pragma once
/**
 * @file LightReferenceResolveSystem.h
 */


 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>
namespace ecs {
	/**
	 * @brief ライトの関連付けを解決するシステム
	 */
	class LightReferenceResolveSystem : public ISystem {
	public:
		//! @brief コンストラクタ
		explicit LightReferenceResolveSystem(const SystemDesc& _desc)
			: ISystem(_desc) {
		}
		//! @brief 初期化
		void Init() override;
		//! @brief 更新
		void Update(float _dt) override;
	private:
	};
}