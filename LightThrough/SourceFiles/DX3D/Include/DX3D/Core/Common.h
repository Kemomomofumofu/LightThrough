#pragma once
/**
 * @file Common.h
 * @brief コモンです
 * @author Arima Keita
 * @date 2025-06-25
 */
/*---------- インクルード ----------*/
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <d3d11.h>
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

	struct GraphicsDeviceDesc {
		BaseDesc base;
	};

	struct SwapChainDesc {
		void* winHandle{};
		Rect winSize{};
	};

	struct VertexShaderSignatureDesc {
		const ShaderBinaryPtr& vsBinary;
	};

	struct BinaryData {
		const void* data{};
		size_t dataSize{};
	};

	/**
	 * @brief インデックスバッファ構造体
	 * 
	 * indexList	: インデックスデータのポインタ
	 * indexCount	: インデックス数
	 */
	struct IndexBufferDesc {
		const uint32_t* indexList{};
		uint32_t indexCount{};
	};

	/**
	 * @brief 頂点バッファ構造体
	 * @details
	 * - \c vertexList		: 頂点データのポインタ
	 * - \c vertexListSize	: 頂点データのバイトサイズ
	 * - \c vertexSize		: 1頂点あたりのバイトサイズ
	 * @todo 頂点バッファの更新方法を指定できるようにする
	 */
	struct VertexBufferDesc {
		const void* vertexList{};
		uint32_t vertexListSize{};
		uint32_t vertexSize{};
	};

	struct ConstantBufferDesc {
		uint32_t byteWidth{};
		const void* initData{};
	};

	struct StructuredBufferDesc {
		uint32_t elementSize{};
		uint32_t elementCount{};
		const void* initData{};
	};

	struct RWStructuredBufferDesc {
		uint32_t elementSize{};
		uint32_t elementCount{};
	};

	struct StagingBufferDesc {
		uint32_t elementSize{};
		uint32_t elementCount{};
	};

	struct GameDesc {
		Rect windowSize{ 1280, 720 };
		Logger::LogLevel logLevel = Logger::LogLevel::Error;
	};
}


namespace scene {
	class SceneManager;
}

namespace ecs {
	class Coordinator;

	struct SystemDesc {
		dx3d::BaseDesc base;
		Coordinator& ecs; // Coordinatorへの参照
		scene::SceneManager& sceneManager; // SceneManagerへの参照
		dx3d::GraphicsEngine& graphicsEngine;
		bool oneShot = false; // 一度だけ実行するシステムか
	};
}