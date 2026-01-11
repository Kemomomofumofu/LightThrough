#pragma once
/**
 * @file MoveDirectionSourceResolveSystem.h
 * @brief MoveDirectionSourceコンポーネントの関連付けを解決するシステム
 */


 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>
namespace ecs {
	/**
	 * @brief MoveDirectionSourceコンポーネントの関連付けを解決するシステム
	 */
	class MoveDirectionSourceResolveSystem : public ISystem {
	public:
		//! @brief コンストラクタ
		explicit MoveDirectionSourceResolveSystem(const SystemDesc& _desc)
			: ISystem(_desc) {}
		//! @brief 初期化
		void Init() override;
		//! @brief 更新
		void Update(float _dt) override;
	private:
	};
}