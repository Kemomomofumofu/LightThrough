/**
 * @file GraphicsEngine.cpp
 * @brief グラフィックスエンジンクラスの定義
 * @author Arima Keita
 * @date 2025-07-11
 */

 /*---------- インクルード ----------*/
#include <fstream>

#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Math/Vec3.h>


/*---------- 名前空間 ----------*/
using namespace dx3d;

dx3d::GraphicsEngine::GraphicsEngine(const GraphicsEngineDesc& _desc)
	: Base(_desc.base)
{
	graphics_device_ = std::make_shared<GraphicsDevice>(GraphicsDeviceDesc{ logger_ });

	auto& device = *graphics_device_;
	device_context_ = device.CreateDeviceContext();

	// デフォルトのシェーダーを読み込み
	constexpr char shaderFilePath[] = "SourceFiles/DX3D/Assets/Shaders/Default.hlsl";
	std::ifstream shaderStream(shaderFilePath);
	if (!shaderStream) {
		DX3DLogThrowError("シェーダーファイルを開くのに失敗");
	}
	std::string shaderFileData{
		std::istreambuf_iterator<char>(shaderStream),
		std::istreambuf_iterator<char>()
	};
	auto shaderSourceCode = shaderFileData.c_str();
	auto shaderSourceCodeSize = shaderFileData.length();

	auto vs = device.CompileShader({ shaderFilePath, shaderSourceCode, shaderSourceCodeSize, "VSMain", ShaderType::VertexShader });
	auto ps = device.CompileShader({ shaderFilePath, shaderSourceCode, shaderSourceCodeSize, "PSMain", ShaderType::PixelShader });
	auto vsSig = device.CreateVertexShaderSignature({ vs });

	pipeline_ = device.CreateGraphicsPipelineState({ *vsSig, *ps });

	// 頂点リストを定義
	const Vertex vertexList[] = {
		{ { -0.5f, -0.5f, 0.0f}, { 1, 0, 0, 1} },
		{ { -0.5f,  0.5f, 0.0f}, { 0, 1, 0, 1} },
		{ {  0.5f,  0.5f, 0.0f}, { 0, 0, 1, 1} },

		{ {  0.5f,  0.5f, 0.0f}, { 0, 0, 1, 1} },
		{ {  0.5f, -0.5f, 0.0f}, { 0, 1, 0, 1} },
		{ { -0.5f, -0.5f, 0.0f}, { 1, 0, 0, 1} }
	};
	vb_ = device.CreateVertexBuffer({ vertexList, std::size(vertexList), sizeof(Vertex) });
}

dx3d::GraphicsEngine::~GraphicsEngine()
{
}

GraphicsDevice& dx3d::GraphicsEngine::GetGraphicsDevice() noexcept
{
	return *graphics_device_;
}

void dx3d::GraphicsEngine::Render(SwapChain& _swapChain)
{
	auto& context = *device_context_;
	context.ClearAndSetBackBuffer(_swapChain, { 0.27f, 0.39f, 0.55f, 1.0f });	// 初期色でクリア
	context.SetGraphicsPipelineState(*pipeline_);

	context.SetViewportSize(_swapChain.GetSize());

	// 頂点バッファ
	auto& vb = *vb_;
	context.SetVertexBuffer(vb);
	context.DrawTriangleList(vb.GetVertexListSize(), 0u);	// テストでの三角形描画

	auto& device = *graphics_device_;
	device.ExecuteCommandList(context);
	_swapChain.Present();
}
