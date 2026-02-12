#pragma once
/**
 * @file MeshRegistry.h
 * @brief メッシュの登録・管理を行うクラス
 */

 // ---------- インクルード ---------- //
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

#include <DX3D/Graphics/Meshes/MeshHandle.h>

namespace dx3d {
	struct Mesh;
	
	class MeshRegistry {
	public:
		using HandleType = uint32_t;

		MeshHandle Register(std::shared_ptr<Mesh> _mesh, const std::string& _name);
		Mesh* Get(const MeshHandle& _handle);
		Mesh* GetByName(const std::string& _name);
		MeshHandle GetHandleByName(const std::string& _name);

	private:
		std::vector<std::shared_ptr<Mesh>> meshes_;	// id->mesh
		std::unordered_map<std::string, MeshHandle> name_to_handle_; // name->id

	};


}