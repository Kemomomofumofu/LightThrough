#pragma once
/**
 * @file PipelineKey.h
 * @brief パイプライン識別キー
 */

#include <cstdint>

namespace dx3d {
	// --- モード列挙 ---
	enum class VertexShaderKind : uint8_t {
		None = 0,
		Default,
		Instanced,
		ShadowMap,
		Fullscreen,
		Max,
	};

	enum class PixelShaderKind : uint8_t {
		None = 0,
		Default,
		Color,
		Outline,
		Max,
	};

	enum class ComputeShaderKind : uint8_t {
		None = 0,
		ShadowTest,
		Max,
	};

	enum class BlendMode : uint8_t {
		Opaque,
		Alpha,
		Add,
		Max,
	};

	enum class DepthMode : uint8_t {
		Default = 0,
		ReadOnly,
		Disable,
		Max,
	};

	enum class RasterMode : uint8_t {
		SolidBack = 0,
		SolidNone,
		Wireframe,
		Max,
	};

	// --- Flags（軽いスイッチ用） ---
	namespace PipelineFlags {
		constexpr uint8_t Instancing = 0x01;
		constexpr uint8_t ShadowPass = 0x02;  // 互換用（今後は使わずモードで決定）
		constexpr uint8_t AlphaTest = 0x04;
	}

	/**
	 * bit構成:
	 *  0-3   : VertexShaderKind
	 *  4-7   : PixelShaderKind
	 *  8-11  : BlendMode
	 *  12-13 : DepthMode   (Max 4)
	 *  14-15 : RasterMode  (Max 4)
	 *  16-22 : Flags (7bit)
	 *  23-31 : Reserved
	 */
	union PipelineKey {
		struct Fields {
			uint32_t vs : 4;
			uint32_t ps : 4;
			uint32_t blend : 4;
			uint32_t depth : 2;
			uint32_t raster : 2;
			uint32_t flags : 7;
			uint32_t reserved : 9;
		} f;
		uint32_t value{ 0 };

		PipelineKey() = default;
		PipelineKey(VertexShaderKind _vs, PixelShaderKind _ps, uint32_t _flags = 0) {
			f.vs = static_cast<uint32_t>(_vs);
			f.ps = static_cast<uint32_t>(_ps);
			f.blend = static_cast<uint32_t>(BlendMode::Opaque);
			f.depth = static_cast<uint32_t>(DepthMode::Default);
			f.raster = static_cast<uint32_t>(RasterMode::SolidBack);
			f.flags = _flags & 0x7F;
			f.reserved = 0;
		}

		bool operator==(const PipelineKey& _o) const noexcept { return value == _o.value; }
		bool operator!=(const PipelineKey& _o) const noexcept { return value != _o.value; }
		bool operator<(const PipelineKey& _o) const noexcept { return value < _o.value; }

		VertexShaderKind GetVS() const noexcept { return static_cast<VertexShaderKind>(f.vs); }
		PixelShaderKind  GetPS() const noexcept { return static_cast<PixelShaderKind>(f.ps); }
		BlendMode        GetBlend() const noexcept { return static_cast<BlendMode>(f.blend); }
		DepthMode        GetDepth() const noexcept { return static_cast<DepthMode>(f.depth); }
		RasterMode       GetRaster() const noexcept { return static_cast<RasterMode>(f.raster); }
		uint32_t         GetFlags() const noexcept { return f.flags; }

		void SetVS(VertexShaderKind _vs) noexcept { f.vs = static_cast<uint32_t>(_vs); }
		void SetPS(PixelShaderKind _ps) noexcept { f.ps = static_cast<uint32_t>(_ps); }
		void SetBlend(BlendMode _blend) noexcept { f.blend = static_cast<uint32_t>(_blend); }
		void SetDepth(DepthMode _depth) noexcept { f.depth = static_cast<uint32_t>(_depth); }
		void SetRaster(RasterMode _raster) noexcept { f.raster = static_cast<uint32_t>(_raster); }
		void SetFlags(uint32_t _flags) noexcept { f.flags = _flags & 0x7F; }
		void AddFlags(uint32_t _flags) noexcept { f.flags = (f.flags | _flags) & 0x7F; }
		void ClearFlags(uint32_t _flags) noexcept { f.flags = (f.flags & ~_flags) & 0x7F; }
	};

	struct PipelineKeyHash {
		size_t operator()(const PipelineKey& _k) const noexcept {
			return static_cast<size_t>(_k.value);
		}
	};

	// --- 互換用（影/通常） ---
	inline PipelineKey BuildPipelineKey(bool _shadowPass, BlendMode _blend = BlendMode::Opaque) noexcept {
		PipelineKey key{};
		key.AddFlags(PipelineFlags::Instancing); // 既存互換

		if (_shadowPass) {
			key.SetVS(VertexShaderKind::ShadowMap);
			key.SetPS(PixelShaderKind::None);
			key.SetBlend(BlendMode::Opaque);
			key.SetDepth(DepthMode::Default);
			key.SetRaster(RasterMode::SolidBack);
		}
		else {
			key.SetVS(VertexShaderKind::Instanced);
			key.SetPS(PixelShaderKind::Default);
			key.SetBlend(_blend);
			key.SetDepth(DepthMode::Default);
			key.SetRaster(RasterMode::SolidBack);
		}
		return key;
	}

	// --- 明示指定版 ---
	inline PipelineKey BuildPipelineKey(
		VertexShaderKind _vs,
		PixelShaderKind  _ps,
		BlendMode        _blend = BlendMode::Opaque,
		DepthMode        _depth = DepthMode::Default,
		RasterMode       _raster = RasterMode::SolidBack,
		uint32_t         _flags = 0) noexcept
	{
		PipelineKey key{};
		key.SetVS(_vs);
		key.SetPS(_ps);
		key.SetBlend(_blend);
		key.SetDepth(_depth);
		key.SetRaster(_raster);
		key.SetFlags(_flags);
		return key;
	}

	// static_assert
	static_assert(static_cast<uint32_t>(VertexShaderKind::Max) <= 16, "[PipelineKey] VertexShaderKind overflow");
	static_assert(static_cast<uint32_t>(PixelShaderKind::Max) <= 16, "[PipelineKey] PixelShaderKind overflow");
	static_assert(static_cast<uint32_t>(BlendMode::Max) <= 16, "[PipelineKey] BlendMode overflow");
	static_assert(static_cast<uint32_t>(DepthMode::Max) <= 4, "[PipelineKey] DepthMode overflow");
	static_assert(static_cast<uint32_t>(RasterMode::Max) <= 4, "[PipelineKey] RasterMode overflow");
}