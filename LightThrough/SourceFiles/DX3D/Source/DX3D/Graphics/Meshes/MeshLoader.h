#pragma once
/**
* @file MeshLoader.h
* @brief メッシュの読み込みを行うクラス
* Assimpを用いて読み込む
* @author Arima Keita
* @date 2025-10-16
*/

// ---------- インクルード ---------- //
#include <memory>
#include <string>
#include <vector>
#include <assimp/scene.h>



namespace dx3d {

	struct Mesh;
	class MeshRegistry;
	class GraphicsDevice;

	class MeshLoader {
	public:
		static void LoadFromFile(const std::string& _path, const std::string& _key, GraphicsDevice& _device, MeshRegistry& _registry);
	};
}	// namespace dx3d