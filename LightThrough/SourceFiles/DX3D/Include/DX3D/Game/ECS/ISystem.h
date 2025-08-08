#pragma once
/**
 * @file ISystem.h
 * @brief システムのインターフェース
 * @author Arima Keita
 * @date 2025-08-08
 */

// ---------- インクルード ---------- // 
#include <set>
#include <DX3D/Game/ECS/Entity.h>

namespace ecs {
	class ISystem {
	public:
		std::set<Entity> entities_;	// 処理対象のEntityの集合
		virtual ~ISystem() = default;
	};
}