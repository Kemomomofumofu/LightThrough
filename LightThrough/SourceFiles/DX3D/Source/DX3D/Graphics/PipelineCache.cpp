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
		auto psBin = GetOrCreatePS(_key.GetPS());

		// パイプラインステートの定義
		GraphicsPipelineStateDesc psoDesc{
			.vs = *vsEntry.signature,
			.ps = *psBin,
			.inputLayout = vsEntry.layout,
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
		case VertexShaderKind::None:		DX3DLogThrowError("[PipelineCache] VS が未割り当て"); break;
		case VertexShaderKind::Default:		file = paths_.vsDefault; break;
		case VertexShaderKind::Instanced:	file = paths_.vsInstanced; break;
		case VertexShaderKind::ShadowMap:	file = paths_.vsSpriteShadow; break;
		default: DX3DLogThrowError("[PipelineCache] 未対応の頂点シェーダー");
		}

		// シェーダーのコンパイル
		auto vsBin = CompileFile(file, "VSMain", ShaderBinary::Type::Vertex);
		auto sig = graphics_device_->CreateVertexShaderSignature({ vsBin });
		InputLayoutPtr layout = nullptr;
		if (_kind == VertexShaderKind::Instanced)
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
		// ファイルパスがnullptrなら、組み込みのダミーシェーダーをコンパイルする
		if (_path == nullptr)
		{
			std::string dummySrc;
			switch (_type)
			{
			case ShaderBinary::Type::Pixel:
				// 黒を返すPS
				dummySrc.reserve(96);
				dummySrc += "float4 ";
				dummySrc += _entry ? _entry : "PSMain";
				dummySrc += "() : SV_Target { return float4(0, 0, 0, 0); }";
				break;
			case ShaderBinary::Type::Vertex:
				// 最小限の頂点シェーダ（固定位置を返す）
				dummySrc.reserve(192);
				dummySrc += "struct VSOut { float4 pos : SV_Position; }; VSOut ";
				dummySrc += _entry ? _entry : "VSMain";
				dummySrc += "(uint vertexId : SV_VertexID) { VSOut o; o.pos = float4(0, 0, 0, 1); return o; }";
				break;
			default:
				DX3DLogThrowError("[PipelineCache] 未対応のシェーダタイプ");
			}

			return graphics_device_->CompileShader(
				{ "DummyShader", dummySrc.c_str(), dummySrc.size(), _entry, _type }
			);
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
