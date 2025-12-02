#pragma once
/**
 * @file PipelineKey.h
 * @brief パイプライン識別キー
 * @author Arima Keita
 * @date 2025-09-27
 */

 /*---------- インクルード ----------*/
#include <cstdint>
#include <tuple>

namespace dx3d {
	/**
	 * @brief 頂点シェーダの種類をまとめた列挙型
	 */
	enum class VertexShaderKind : uint8_t {
		None = 0,
		Default,
		Instanced,
		ShadowMap,
		Max,
	};

	/**
	 * @brief ピクセルシェーダの種類をまとめた列挙型
	 */
	enum class PixelShaderKind : uint8_t {
		None = 0,
		Default,
		Max,
	};

	//! brief ブレンドモード
	enum class BlendMode : uint8_t {
		Opaque,
		Alpha,
		Add,
		Max
	};


	// フラグビットの定義 [ToDo] 必要になったら拡張
	namespace PipelineFlags {
		constexpr uint8_t Instancing = 0x01;	// インスタンシング有効
		constexpr uint8_t ShadowPass = 0x02;	// シャドウマップパス
		constexpr uint8_t AlphaTest = 0x04;	// アルファテスト有効

	}

	/**
	 * @brief パイプライン識別キー
	 *
	 * パイプラインステートオブジェクトを識別するためのキー。
	 * 頂点シェーダ、ピクセルシェーダ、フラグビットなどを組み合わせて一意に識別する。
	 * 32bitに収まるように設計されている。
	 *
	 * bit構成:
	 * 0-3: VertexShaderKind (Max 16種類)
	 * 4-7: PixelShaderKind (Max 16種類)
	 * 8-11: BlendMode (Max 16種類)
	 * 12-18: Flags (7bit)
	 * 19-31: Reserved (13bit) 拡張用
	 */
	union PipelineKey {
		struct Fields {
			uint32_t vs : 4;
			uint32_t ps : 4;
			uint32_t blend : 4;
			// uint32_t depth : 3;	// [ToDo] 将来必要になりそう
			// uint32_t raster : 3;
			// uint32_t topology : 3;
			uint32_t flags : 3;
			uint32_t reserved : 17;
		} f;
		uint32_t value{ 0 };

		PipelineKey() = default;
		PipelineKey(VertexShaderKind _vs, PixelShaderKind _ps, uint32_t _flags = 0) {
			f.vs = static_cast<uint32_t>(_vs);
			f.ps = static_cast<uint32_t>(_ps);
			f.blend = static_cast<uint32_t>(BlendMode::Opaque);
			f.flags = _flags & 0x7F;
			f.reserved = 0;
		}

		bool operator==(const PipelineKey& _o)const noexcept { return value == _o.value; }
		bool operator!=(const PipelineKey& _o)const noexcept { return value != _o.value; }
		bool operator<(const PipelineKey& _o)const noexcept { return value < _o.value; }

		VertexShaderKind GetVS() const noexcept { return static_cast<VertexShaderKind>(f.vs); }
		PixelShaderKind  GetPS() const noexcept { return static_cast<PixelShaderKind>(f.ps); }
		BlendMode        GetBlend() const noexcept { return static_cast<BlendMode>(f.blend); }
		uint32_t GetFlags() const noexcept { return f.flags; }

		void SetVS(VertexShaderKind _vs) noexcept { f.vs = static_cast<uint32_t>(_vs); }
		void SetPS(PixelShaderKind _ps) noexcept { f.ps = static_cast<uint32_t>(_ps); }
		void SetDepthMode(uint32_t _depth) noexcept { /*f.depth = _depth & 0x7;*/ }
		void SetBlend(BlendMode _blend) noexcept { f.blend = static_cast<uint32_t>(_blend); }
		void SetFlags(uint32_t _flags) noexcept { f.flags = _flags & 0x7F; }
		void AddFlags(uint32_t _flags) noexcept { f.flags = (f.flags | _flags) & 0x7F; }
		void ClearFlags(uint32_t _flags) noexcept { f.flags = (f.flags & ~_flags) & 0x7F; }
	};

	// ハッシュ関数
	struct PipelineKeyHash {
		size_t operator()(const PipelineKey& _k)const noexcept {
			return static_cast<size_t>(_k.value);
		}
	};


	//! @brief パイプラインキーの構築
	inline PipelineKey BuildPipelineKey(bool _shadowPass, BlendMode _blend = BlendMode::Opaque) noexcept {
		PipelineKey key{};

		key.AddFlags(PipelineFlags::Instancing);

		if (_shadowPass) {
			key.SetVS(VertexShaderKind::ShadowMap);
			key.SetPS(PixelShaderKind::None);
			key.AddFlags(PipelineFlags::ShadowPass);
		}
		else {
			key.SetVS(VertexShaderKind::Instanced);
			key.SetPS(PixelShaderKind::Default);
			key.SetBlend(_blend);

		}

		return key;
	}

	// 定義の静的アサート
	static_assert(static_cast<uint32_t>(VertexShaderKind::Max) <= 16, "[PipelineKey] VertexShaderKindの種類が多すぎます");
	static_assert(static_cast<uint32_t>(PixelShaderKind::Max) <= 16, "[PipelineKey] PixelShaderKindの種類が多すぎます");
	static_assert(static_cast<uint32_t>(BlendMode::Max) <= 16, "[PipelineKey] BlendModeの種類が多すぎます");
	
}

// 将来必要になりそうなものをメモ
//enum class DepthMode : uint8_t { Default, ReadOnly, Disable, Max };
//enum class RasterMode : uint8_t { SolidBack, SolidNone, Wireframe, Max };
//enum class TopologyKind : uint8_t { Triangles, Lines, Points, Max };
//
//struct PipelineKey {
//	VertexShaderKind vs;
//	PixelShaderKind  ps;
//	BlendMode        blend;
//	DepthMode        depth;
//	RasterMode       raster;
//	TopologyKind     topology;
//	uint8_t          flags;
//};