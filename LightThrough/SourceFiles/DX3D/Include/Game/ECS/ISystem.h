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
#include <Game/GameLogUtils.h>

namespace ecs {
	// ---------- 前方宣言 ---------- //
	struct Entity;
	class Coordinator;


	class ISystem  : public dx3d::Base{
	public:
		explicit ISystem(const SystemDesc& _desc)
			: dx3d::Base(_desc.base)
			, ecs_(_desc.ecs){}

		virtual ~ISystem() = default;
		virtual void Update(float _dt) {}
		virtual void FixedUpdate(float _fixedDt) {}

		std::set<Entity> entities_;	// 処理対象のEntityの集合

	protected:
		::ecs::Coordinator& ecs_; // ecs::Coordinatorへの参照
	};
}