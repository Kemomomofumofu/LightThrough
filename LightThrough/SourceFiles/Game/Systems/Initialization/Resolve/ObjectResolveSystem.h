#pragma once

/**
 * @file ObjectResolveSystem.h
 * @brief オブジェクト同士の関連付けを解決するシステム
 */


 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>

namespace ecs {
	/**
	 * @brief オブジェクト同士の関連付けを解決するシステム
	 */
	class ObjectResolveSystem : public ISystem {
	public:
		//! @brief コンストラクタ
		explicit ObjectResolveSystem(const SystemDesc& _desc)
			: ISystem(_desc) {}

		//! @brief 初期化
		void Init() override;

		//! @brief 更新
		void Update(float _dt) override;
	private:
	};
}