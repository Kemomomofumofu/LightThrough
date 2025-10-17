/**
 * @file MeshRegistry.cpp
 * @brief メッシュの登録・管理を行うクラス
 * @author Arima Keita
 * @date 2025-10-16
 */

 // ---------- インクルード ---------- //
#include <DX3D/Graphics/Meshes/MeshRegistry.h>
#include <DX3D/Graphics/Meshes/Mesh.h>
#include <Debug/Debug.h>

namespace dx3d {

    /**
     * @brief メッシュを登録する
     * @param _mesh 登録するメッシュ
     * @param _name メッシュの名前(重複不可)
     * @return 登録されたメッシュのハンドル
	 */
    MeshHandle MeshRegistry::Register(std::shared_ptr<Mesh> _mesh, const std::string& _name)
    {
        // 新しいIDを生成(0は無効値として取っておく)
        HandleType newId = static_cast<HandleType>(meshes_.size() + 1); 
        meshes_.push_back(std::move(_mesh));
        MeshHandle handle{ newId };
        name_to_handle_[_name] = handle;

        return handle;
    }

    /**
     * @brief ハンドルからメッシュを取得する
     * @param _handle 取得するメッシュのハンドル
     * @return 有る: メッシュのポインタ, 無い: nullptr
	 */
    Mesh* MeshRegistry::Get(const MeshHandle& _handle)
    {
        if (!_handle.IsValid()) {
            return nullptr;
        }

        const size_t index = static_cast<size_t>(_handle.id - 1);

		// IDが範囲内かチェック
        if (index < meshes_.size()) {
			return meshes_[index].get();
        }

		// 範囲外ならnullptrを返す
        return nullptr;
    }

    /**
     * @brief 名前からメッシュを取得する
     * @param _name 取得するメッシュの名前
	 * @return 有る: メッシュのポインタ, 無い: nullptr
     */
    Mesh* MeshRegistry::GetByName(const std::string& _name)
    {
        auto it = name_to_handle_.find(_name);
        // 見つからない場合
        if (it == name_to_handle_.end()) { return nullptr; }

		// 見つかった場合
        return Get(it->second);
    }

    /**
     * @brief 名前からメッシュハンドルを取得する
     * @param _name 取得するメッシュの名前
	 * @return 有る: メッシュハンドル, 無い: 無効なメッシュハンドル
     */
    MeshHandle MeshRegistry::GetHandleByName(const std::string& _name)
    {
        auto it = name_to_handle_.find(_name);

        if (it != name_to_handle_.end()) {
            return it->second;
        }

        
		return dx3d::MeshHandle{}; // 無効なハンドルを返す
    }



}   // namespace dx3d
