#pragma once
/**
 * @file ISystem.h
 * @brief Updateを持つシステムのインターフェース
 * @author Arima Keita
 * @date 2025-08-08
 */

// ---------- インクルード ---------- // 
#include <set>
#include <DX3D/Core/Base.h>
#include <DX3D/Game/ECS/ECSLogUtils.h>

namespace ecs {
	// ---------- 前方宣言 ---------- //
	struct Entity;
	class Coordinator;

	class ISystem  : public dx3d::Base{
	public:
		explicit ISystem(const dx3d::SystemDesc& _desc) : dx3d::Base(_desc.base){}
		virtual ~ISystem() = default;
		virtual void Update(float _dt, Coordinator& _ecs) = 0;

		std::set<Entity> entities_;	// 処理対象のEntityの集合
	};
}