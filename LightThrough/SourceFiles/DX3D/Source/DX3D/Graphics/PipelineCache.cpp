/**
 * @file PipelineCache.h
 * @brief パイプラインをキャッシュするクラス
 * @author Arima Keita
 * @date 2025-09-27
 */

 // ---------- インクルード ---------- //
#include <wrl/client.h>
#include <fstream>
#include <filesystem>
#include <DX3D/Graphics/PipelineCache.h>
#include <DX3D/Graphics/GraphicsDevice.h>


namespace dx3d {
	/**
	 * @brief パイプラインステートオブジェクトを取得または生成
	 * @param _key	パイプラインのキー
	 * @return パイプラインステートオブジェクトのポインタ
	 */
	GraphicsPipelineStatePtr PipelineCache::GetOrCreate(const PipelineKey& _key)
	{
		// 既に存在するならそれを返す
		if (auto it = pso_cache_.find(_key); it != pso_cache_.end()) { return it->second; }


		auto& vsEntry = GetOrCreateVS(_key.GetVS());
		ShaderBinaryPtr psBin = nullptr;
		if ((_key.GetFlags() & PipelineFlags::ShadowPass) == 0) {
			psBin = GetOrCreatePS(_key.GetPS());
		}
		// パイプラインステートの定義
		GraphicsPipelineStateDesc psoDesc{
			.vs = *vsEntry.signature,
			.ps = psBin.get(),
			.inputLayout = vsEntry.layout,
			.blendMode = _key.GetBlend()
		};

		// パイプラインステートオブジェクトの生成
		auto pso = graphics_device_->CreateGraphicsPipelineState(psoDesc);

		// キャッシュに保存してから返す
		pso_cache_.emplace(_key, pso);
		return pso;
	}

	/**
	 * @brief ピクセルシェーダーを取得または生成
	 * @param _kind	ピクセルシェーダーの種類
	 * @return シェーダーバイナリのポインタ
	 */
	PipelineCache::VSCacheEntry& PipelineCache::GetOrCreateVS(VertexShaderKind _kind)
	{
		// 既に存在するならそれを返す
		if (auto it = vs_cache_.find(_kind); it != vs_cache_.end()) { return it->second; }

		const char* file = nullptr;
		switch (_kind) {
		case VertexShaderKind::None:		file = nullptr; break;
		case VertexShaderKind::Default:		file = paths_.vsDefault; break;
		case VertexShaderKind::Instanced:	file = paths_.vsInstanced; break;
		case VertexShaderKind::ShadowMap:	file = paths_.vsSpriteShadow; break;
		default: DX3DLogThrowError("[PipelineCache] 未対応の頂点シェーダー");
		}

		// シェーダーのコンパイル
		auto vsBin = CompileFile(file, "VSMain", ShaderBinary::Type::Vertex);
		auto sig = graphics_device_->CreateVertexShaderSignature({ vsBin });
		InputLayoutPtr layout = nullptr;
		if (_kind == VertexShaderKind::Instanced || _kind == VertexShaderKind::ShadowMap)
		{
			layout = graphics_device_->CreateInputLayout({ sig, "INSTANCE_" });
		}
		else {
			layout = graphics_device_->CreateInputLayout({ sig });
		}
		// キャッシュに保存してから返す
		VSCacheEntry entry{ vsBin, sig, layout };
		auto [pos, inserted] = vs_cache_.emplace(_kind, std::move(entry));
		return pos->second;
	}

	/**
	 * @brief ピクセルシェーダーを取得または生成
	 * @param _kind	ピクセルシェーダーの種類
	 * @return シェーダーバイナリのポインタ
	 */
	ShaderBinaryPtr PipelineCache::GetOrCreatePS(PixelShaderKind _kind)
	{

		// 既に存在するならそれを返す
		if (auto it = ps_cache_.find(_kind); it != ps_cache_.end()) { return it->second; }

		const char* file = nullptr;

		switch (_kind) {
		case PixelShaderKind::None: file = nullptr; break;
		case PixelShaderKind::Default: file = paths_.psDefault; break;
		default: DX3DLogThrowError("[PipelineCache] 未対応のピクセルシェーダー");
		}

		// シェーダーのコンパイル
		auto psBin = CompileFile(file, "PSMain", ShaderBinary::Type::Pixel);
		// キャッシュに保存してから返す
		ps_cache_.emplace(_kind, psBin);
		return psBin;
	}

	/**
	 * @brief コンパイル
	 * @param _path		ファイルパス
	 * @param _entry	エントリポイント
	 * @param _type		Shaderの種類
	 * @return ShaderBinaryのポインタ
	 */
	ShaderBinaryPtr PipelineCache::CompileFile(const char* _path, const char* _entry, ShaderBinary::Type _type)
	{
		// パスが無い場合はダミーシェーダー
		if (_path == nullptr)
		{
			const char* srcVS =
				"float4 VSMain(uint id : SV_VertexID) : SV_Position { return float4(0,0,0,1); }";

			const char* srcPS =
				"float4 PSMain() : SV_Target { return float4(0,0,0,1); }";

			const char* src = nullptr;

			// どちらの種類か判定
			if (_type == ShaderBinary::Type::Vertex) {
				src = srcVS;
			}
			else {
				src = srcPS;
			}

			// そのままコンパイル
			return graphics_device_->CompileShader({
				"<dummy>",          // パスの代わり（ログ用）
				src,                // ソース
				std::strlen(src),
				_entry,             // VSMain or PSMain
				_type
				});
		}


		// ファイルの読み込み
		auto src = LoadTextFile(_path);
		// コンパイル
		return graphics_device_->CompileShader({ _path, src.c_str(), src.size(), _entry, _type });
	}

	/**
	 * @brief テキストファイルを読み込む
	 * @param _path		ファイルパス
	 * @return ファイルの文字列
	 */
	std::string PipelineCache::LoadTextFile(const char* path) {
		std::ifstream ifs(path);
		if (!ifs) {
			DX3DLogThrowError("[PipelineCache] シェーダファイルを開けません");
		}
		return { std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };
	}
}
