/**
 * @file GraphicsEngine.cpp
 * @brief グラフィックスエンジンクラスの定義
 * @author Arima Keita
 * @date 2025-07-11
 */

 /*---------- インクルード ----------*/
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
	graphics_device_ = std::make_shared<GraphicsDevice>(GraphicsDeviceDesc{logger_});

	auto& device = *graphics_device_;
	device_context_ = device.CreateDeviceContext();

	// デフォルトのシェーダーを設定
	constexpr char shaderSourceCode[] =
		R"(
float4 VSMain(float3 pos: POSITION):SV_Position
{
return float4(pos.xyz, 1.0);
}
float PSMain(): SV_Target
{
return float4(1.0, 1.0, 1.0, 1.0);
}
)";

	constexpr char shaderSourceName[] = "Default";
	constexpr auto shaderSourceCodeSize = std::size(shaderSourceCode);

	auto vs = device.CompileShader({ shaderSourceName, shaderSourceCode, shaderSourceCodeSize, "VSMain", ShaderType::VertexShader });
	auto ps = device.CompileShader({ shaderSourceName, shaderSourceCode, shaderSourceCodeSize, "PSMain", ShaderType::PixelShader });

	pipeline_ = device.CreateGraphicsPipelineState({ *vs, *ps });

	const Vec3 vertexList[] = {
		{-0.5f, -0.5f, 0.0f},
		{ 0.0f,  0.5f, 0.0f},
		{ 0.5f, -0.5f, 0.0f}
	};

	vb_ = device.CreateVertexBuffer({ vertexList, std::size(vertexList), sizeof(Vec3) });
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
	context.ClearAndSetBackBuffer(_swapChain, { 1, 0, 0, 1 });
	context.SetGraphicsPipelineState(*pipeline_);

	context.SetViewportSize(_swapChain.GetSize());

	auto& vb = *vb_;
	context.SetVertexBuffer(vb);
	context.DrawTriangleList(vb.GetVertexListSize(), 0u);

	auto& device = *graphics_device_;
	device.ExecuteCommandList(context);
	_swapChain.Present();
}
