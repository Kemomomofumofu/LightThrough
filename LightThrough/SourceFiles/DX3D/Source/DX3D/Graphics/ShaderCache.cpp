/**
 * @file ShaderCache.cpp
 * @brief シェーダーをキャッシュするクラス
 * @author Arima Keita
 * @date 2025-12-11
 */

 // ---------- インクルード ---------- //
#include <wrl/client.h>
#include <fstream>
#include <DX3D/Graphics/ShaderCache.h>
#include <DX3D/Graphics/GraphicsDevice.h>

namespace dx3d {

	ShaderCache::VSEntry& ShaderCache::GetVS(VertexShaderKind _kind)
	{
		if (auto it = vs_cache_.find(_kind); it != vs_cache_.end()) { return it->second; }

		const char* file = nullptr;
		switch (_kind) {
		case VertexShaderKind::None:      file = nullptr; break;
		case VertexShaderKind::Default:   file = paths_.vsDefault; break;
		case VertexShaderKind::Instanced: file = paths_.vsInstanced; break;
		case VertexShaderKind::ShadowMap: file = paths_.vsShadow; break;
		default: DX3DLogThrowError("[ShaderCache] 未対応の頂点シェーダー");
		}

		auto vsBin = CompileFile(file, "VSMain", ShaderBinary::Type::Vertex);
		auto sig = graphics_device_->CreateVertexShaderSignature({ vsBin });

		InputLayoutPtr layout = nullptr;
		if (_kind == VertexShaderKind::Instanced || _kind == VertexShaderKind::ShadowMap) {
			layout = graphics_device_->CreateInputLayout({ sig, "INSTANCE_" });
		}
		else {
			layout = graphics_device_->CreateInputLayout({ sig });
		}

		VSEntry entry{ vsBin, sig, layout };
		auto [pos, inserted] = vs_cache_.emplace(_kind, std::move(entry));
		return pos->second;
	}

	ShaderBinaryPtr ShaderCache::GetPS(PixelShaderKind _kind)
	{
		if (auto it = ps_cache_.find(_kind); it != ps_cache_.end()) { return it->second; }

		const char* file = nullptr;
		switch (_kind) {
		case PixelShaderKind::None:    file = nullptr; break;
		case PixelShaderKind::Default: file = paths_.psDefault; break;
		case PixelShaderKind::Color: file = paths_.psColor; break;
		default: DX3DLogThrowError("[ShaderCache] 未対応のピクセルシェーダー");
		}

		auto psBin = CompileFile(file, "PSMain", ShaderBinary::Type::Pixel);
		ps_cache_.emplace(_kind, psBin);
		return psBin;
	}

	ShaderCache::CSEntry& ShaderCache::GetCS(ComputeShaderKind _kind)
	{
		if (auto it = cs_cache_.find(_kind); it != cs_cache_.end()) { return it->second; }

		const char* file = nullptr;
		switch (_kind) {
		case ComputeShaderKind::None:       file = nullptr; break;
		case ComputeShaderKind::ShadowTest: file = paths_.csShadowTest; break;
		default: DX3DLogThrowError("[ShaderCache] 未対応のコンピュートシェーダー");
		}

		auto csBin = CompileFile(file, "CSMain", ShaderBinary::Type::Compute);

		Microsoft::WRL::ComPtr<ID3D11ComputeShader> cs{};
		if (csBin) {
			auto data = csBin->GetData();
			DX3DGraphicsLogThrowOnFail(
				device_.CreateComputeShader(data.data, data.dataSize, nullptr, &cs),
				"[ShaderCache] コンピュートシェーダーの生成に失敗"
			);
		}

		CSEntry entry{ csBin, cs };
		auto [pos, inserted] = cs_cache_.emplace(_kind, std::move(entry));
		return pos->second;
	}

	ShaderBinaryPtr ShaderCache::CompileFile(const char* _path, const char* _entry, ShaderBinary::Type _type)
	{
		if (_path == nullptr) { return nullptr; }

		auto src = LoadTextFile(_path);
		return graphics_device_->CompileShader({ _path, src.c_str(), src.size(), _entry, _type });
	}

	std::string ShaderCache::LoadTextFile(const char* _path)
	{
		std::ifstream ifs(_path);
		if (!ifs) {
			DX3DLogThrowError("[ShaderCache] シェーダファイルを開けません");
		}
		return { std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };
	}

} // namespace dx3d