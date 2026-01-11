#pragma once

/**
 * @file Material.h
 * @brief マテリアルコンポーネント
 * @author Arima Keita
 * @date 2025-09-27
 */

 // ---------- インクルード ---------- //
#include <DX3D/Graphics/Material.h>

namespace ecs {
	struct Material {
		std::shared_ptr<dx3d::Material> material{};
	};
}
