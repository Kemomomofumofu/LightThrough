/**
 * @file MeshLoader.cpp
 * @brief メッシュの読み込みを行うクラス
 * @author Arima Keita
 * @date 2025-10-16
 */


 // ---------- インクルード ---------- //
#include <DX3D/Graphics/Meshes/MeshLoader.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/Meshes/MeshRegistry.h>
#include <DX3D/Graphics/Meshes/Mesh.h>
#include <DX3D/Graphics/Buffers/Vertex.h>

namespace dx3d {

	void MeshLoader::LoadFromFile(const std::string& _path, const std::string& _key, GraphicsDevice& _device, MeshRegistry& _registry)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			_path,
			aiProcess_Triangulate |
			aiProcess_GenNormals |
			aiProcess_JoinIdenticalVertices |
			aiProcess_CalcTangentSpace
		);

		if (!scene || !scene->HasMeshes()) {
			throw std::runtime_error("メッシュの読み込みに失敗: " + _path);
		}

		// 最初のメッシュだけ読み込む
		aiMesh* aimesh = scene->mMeshes[0];

		// 頂点データの取得
		std::vector<Vertex> vertices;
		vertices.reserve(aimesh->mNumVertices);
		for (unsigned int i = 0; i < aimesh->mNumVertices; ++i) {
			Vertex v{};
			v.position = { aimesh->mVertices[i].x, aimesh->mVertices[i].y, aimesh->mVertices[i].z };
			if (aimesh->HasNormals()) {
				v.normal = { aimesh->mNormals[i].x, aimesh->mNormals[i].y, aimesh->mNormals[i].z };
			}
			if (aimesh->HasTextureCoords(0)) {
				v.uv = { aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y };
			}
			vertices.push_back(v);
		}
		// インデックスデータの取得
		std::vector<uint32_t> indices;
		for (unsigned int i = 0; i < aimesh->mNumFaces; ++i) {
			aiFace face = aimesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j) {
				indices.push_back(face.mIndices[j]);
			}
		}



		// Bufferの作成, Meshへ格納
		auto mesh = std::make_shared<Mesh>();
		mesh->vb = _device.CreateVertexBuffer({
			vertices.data(),
			static_cast<uint32_t>(vertices.size() * sizeof(Vertex)),
			static_cast<uint32_t>(sizeof(Vertex))
			});
		mesh->ib = _device.CreateIndexBuffer({ indices.data(), static_cast<uint32_t>(indices.size()) });
		mesh->indexCount = static_cast<uint32_t>(indices.size());

		// メッシュの登録
		_registry.Register(mesh, _key);
	}
}	// namespace dx3d