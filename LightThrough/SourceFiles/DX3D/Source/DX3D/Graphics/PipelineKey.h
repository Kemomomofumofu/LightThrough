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
		Default = 0,
		Instanced,
		Max,
	};

	/**
	 * @brief ピクセルシェーダの種類をまとめた列挙型
	 */
	enum class PixelShaderKind : uint8_t {
		Default = 0,
		Max,
	};


	// フラグビットの定義 [ToDo] 必要になったら拡張
	namespace PipelineFlags {
		constexpr uint8_t Instancing = 0x01;	// インスタンシング有効
		// constexpr uint8_t Wireframw = 0x02;	// ワイヤーフレームモード
		// constexpr uint8_t DisableDepth = 0x04;	// 深度テスト無効
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
	 * 8-14: Flags (7bit)
	 * 15-31: Reserved (17bit) 拡張用
	 */
	union PipelineKey {
		struct Fields {
			uint32_t vs : 4;
			uint32_t ps : 4;
			// uint32_t blend : 4;	// [ToDo] 将来必要になりそう
			// uint32_t depth : 3;
			// uint32_t raster : 3;
			// uint32_t topology : 3;
			uint32_t flags : 7;
			uint32_t reserved : 17;
		} f;
		uint32_t value{ 0 };

		PipelineKey() = default;
		PipelineKey(VertexShaderKind _vs, PixelShaderKind _ps, uint32_t _flags = 0) {
			f.vs = static_cast<uint32_t>(_vs);
			f.ps = static_cast<uint32_t>(_ps);
			f.flags = _flags & 0x7F;
			f.reserved = 0;
		}

		bool operator==(const PipelineKey& _o)const noexcept { return value == _o.value; }
		bool operator!=(const PipelineKey& _o)const noexcept { return value != _o.value; }
		bool operator<(const PipelineKey& _o)const noexcept { return value < _o.value; }

		VertexShaderKind GetVS() const noexcept { return static_cast<VertexShaderKind>(f.vs); }
		PixelShaderKind  GetPS() const noexcept { return static_cast<PixelShaderKind>(f.ps); }
		uint32_t GetFlags() const noexcept { return f.flags; }

		void SetVS(VertexShaderKind _vs) noexcept { f.vs = static_cast<uint32_t>(_vs); }
		void SetPS(PixelShaderKind _ps) noexcept { f.ps = static_cast<uint32_t>(_ps); }
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

	// ヘルパー
	/**
	 * @brief インスタンシングフラグの更新
	 * @param _key 更新するパイプラインキー
	 * @param instanceCount インスタンス数
	 */
	inline void PromoteInstancing(PipelineKey& _key, uint32_t instanceCount) noexcept {
		// インスタンスの数が1より大きければ
		if (instanceCount > 1) {
			// インスタンス描画に切り替え
			_key.SetVS(VertexShaderKind::Instanced);
			_key.AddFlags(PipelineFlags::Instancing);
		}
	}

	// 定義の静的アサート
	static_assert(static_cast<uint32_t>(VertexShaderKind::Max) <= 16, "[PipelineKey] VertexShaderKindの種類が多すぎます");
	static_assert(static_cast<uint32_t>(PixelShaderKind::Max) <= 16, "[PipelineKey] PixelShaderKindの種類が多すぎます");
}

// 将来必要になりそうなものをメモ
//enum class BlendMode : uint8_t { Opaque, Alpha, Add, Max };
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