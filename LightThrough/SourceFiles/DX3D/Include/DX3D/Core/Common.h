#pragma once
/**
 * @file Common.h
 * @brief コモンです
 * @author Arima Keita
 * @date 2025-06-25
 */
/*---------- インクルード ----------*/
#include <DX3D/Core/Core.h>
#include <DX3D/Core/Logger.h>
#include <DX3D/Math/Rect.h>

namespace dx3d {
	struct BaseDesc {
		Logger& logger;
	};

	struct WindowDesc {
		BaseDesc base;
		Rect size{};
	};

	struct DisplayDesc {
		WindowDesc window;
		GraphicsDevice& graphicsDevice;
	};

	struct GraphicsEngineDesc {
		BaseDesc base;
	};

	struct GraphicsDeviceDesc {
		BaseDesc base;
	};

	struct SwapChainDesc {
		void* winHandle{};
		Rect winSize{};
	};

	enum class ShaderType {
		VertexShader = 0,
		PixelShader
	};
	
	struct ShaderCompileDesc {
		const char* shaderSourceName{};
		const  void* shaderSourceCode{};
		size_t shaderSourceCodeSize{};
		const char* shaderEntryPoint{};
		ShaderType shaderType{};
	};

	struct VertexShaderSignatureDesc {
		const ShaderBinaryPtr& vsBinary;
	};

	struct BinaryData {
		const void* data{};
		size_t dataSize{};
	};

	struct GraphicsPipelineStateDesc {
		const VertexShaderSignature& vs;
		const ShaderBinary& ps;
	};

	enum class FillMode {
		Solid,
		Wireframe,
	};
	enum class CullMode {
		None,
		Front,
		Back,
	};
	struct RasterizerStateDesc {
		FillMode fillMode = FillMode::Solid;
		CullMode cullMode = CullMode::Back;
		bool frontCounterClockwise = true;
		bool depthClipEnable = true;
		bool scissorEnable = false;
		bool multiSampleEnable = false;
		bool antiAliasedLineEnable = false;
	};

	struct IndexBufferDesc {
		const ui32* indexList{};
		ui32 indexCount{};
	};

	struct VertexBufferDesc {
		const void* vertexList{};
		ui32 vertexListSize{};
		ui32 vertexSize{};
	};

	struct ConstantBufferDesc {
		ui32 byteWidth{};
		const void* initData{};
	};

	struct GameDesc {
		Rect windowSize{ 1280, 720 };
		Logger::LogLevel logLevel = Logger::LogLevel::Error;
	};

	struct SystemDesc {
		dx3d::BaseDesc base;
	};
}