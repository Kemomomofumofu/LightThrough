#pragma once
/**
 * @file Core.h
 * @brief 前方宣言をいっぱいする
 */

 /*---------- インクルード ----------*/
#include <memory>


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
	class ConstantBuffer;
	class StructuredBuffer;
	class RWStructuredBuffer;
	class StagingBuffer;
	class VertexShaderSignature;
	class InputLayout;
	class Texture;
	class PipelineCache;

	using SwapChainPtr = std::shared_ptr<SwapChain>;
	using DeviceContextPtr = std::shared_ptr<DeviceContext>;
	using ShaderBinaryPtr = std::shared_ptr<ShaderBinary>;
	using GraphicsPipelineStatePtr = std::shared_ptr<GraphicsPipelineState>;
	using VertexBufferPtr = std::shared_ptr<VertexBuffer>;
	using IndexBufferPtr = std::shared_ptr<IndexBuffer>;
	using ConstantBufferPtr = std::shared_ptr<ConstantBuffer>;
	using StructuredBufferPtr = std::shared_ptr<StructuredBuffer>;
	using RWStructuredBufferPtr = std::shared_ptr<RWStructuredBuffer>;
	using StagingBufferPtr = std::shared_ptr<StagingBuffer>;
	using VertexShaderSignaturePtr = std::shared_ptr<VertexShaderSignature>;
	using InputLayoutPtr = std::shared_ptr<InputLayout>;
	using TexturePtr = std::shared_ptr<Texture>;
}