#pragma once

/**
 * @file StatickMesh.h
 * @brief 四角や丸などよくある形のオブジェクトのデータ
 * @author Arima Keita
 * @date 2025-08-20
 */

// ---------- インクルード ---------- // 
#include <string>

namespace ecs {
	struct StaticMesh {
		std::string mesh_name_;		// メッシュの名前
		std::string material_name_;	// マテリアルの名前
		bool visible_ = true;		// 可視かどうか
	};

}