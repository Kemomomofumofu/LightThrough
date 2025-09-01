#pragma once
/**
 * @file ISystem.h
 * @brief Updateを持つシステムのインターフェース
 * @author Arima Keita
 * @date 2025-08-08
 */

// ---------- インクルード ---------- // 
#include <set>

namespace ecs {
	// ---------- 前方宣言 ---------- //
	class Entity;		// Entityクラス
	class Coordinator;	// コーディネーター

	class ISystem {
	public:
		std::set<Entity> entities_;	// 処理対象のEntityの集合
		virtual ~ISystem() = default;
		virtual void Update(float _dt, Coordinator& _ecs) = 0;

	};
}