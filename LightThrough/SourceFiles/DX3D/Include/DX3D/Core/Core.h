#pragma once
/**
 * @file Core.h
 * @brief 前方宣言をいっぱいしてくれる
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <stdexcept>
#include <memory>


/**
 * @brief 前方宣言をまとめたファイル
 *
 * クラスの前方宣言をまとめてちゃうファイル。
 */
namespace dx3d {
	class Base;
	class Window;
	class Game;
	class GraphicsEngine;
	class GraphicsDevice;
	class Logger;
	class SwapChain;
	class Display;
	class DeviceContext;
	class ShaderBinary;
	class GraphicsPipelineState;
	class VertexBuffer;
	class IndexBuffer;
	class VertexShaderSignature;

	using i32 = std::int32_t;
	using ui32 = std::uint32_t;
	using f32 = float;
	using d64 = double;

	using SwapChainPtr = std::shared_ptr<SwapChain>;
	using DeviceContextPtr = std::shared_ptr<DeviceContext>;
	using ShaderBinaryPtr = std::shared_ptr<ShaderBinary>;
	using GraphicsPipelineStatePtr = std::shared_ptr<GraphicsPipelineState>;
	using VertexBufferPtr = std::shared_ptr<VertexBuffer>;
	using IndexBufferPtr = std::shared_ptr<IndexBuffer>;
	using VertexShaderSignaturePtr = std::shared_ptr<VertexShaderSignature>;
}