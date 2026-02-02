#pragma once
/**
 * @file MeshHandleResolveSystem.h.h
 */


 // ---------- インクルード ---------- //
#include <Game/ECS/ISystem.h>
#include <DX3D/Graphics/Meshes/MeshRegistry.h>


namespace ecs {
	/**
	 * @brief Meshの関連付けを解決するシステム
	 */
	class MeshHandleResolveSystem : public ISystem {
	public:
		//! @brief コンストラクタ
		explicit MeshHandleResolveSystem(const SystemDesc& _desc)
			: ISystem(_desc)
			, mesh_registry_(_desc.meshRegistry)
		{
		}
		//! @brief 初期化
		void Init() override;
		//! @brief 更新
		void Update(float _dt) override;
	private:
		dx3d::MeshRegistry& mesh_registry_;
	};
}