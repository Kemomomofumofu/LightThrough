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
#include <DX3D/Graphics/Buffers/VertexBuffer.h>
#include <DX3D/Graphics/Buffers/IndexBuffer.h>
#include <DX3D/Math/Vec3.h>

/*---------- 名前空間 ----------*/
using namespace dx3d;

dx3d::GraphicsEngine::GraphicsEngine(const GraphicsEngineDesc& _desc)
	: Base(_desc.base)
{
	graphics_device_ = std::make_shared<GraphicsDevice>(GraphicsDeviceDesc{ logger_ });

	auto& device = *graphics_device_;
	device_context_ = device.CreateDeviceContext();

	// デフォルトの頂点シェーダ読み込み
	constexpr char vsFilePath[] = "SourceFiles/DX3D/Assets/Shaders/Vertex/VS_Instancing.hlsl";
	std::ifstream vsStream(vsFilePath);
	if (!vsStream) {
		DX3DLogThrowError("VSファイルを開くのに失敗");
	}
	// シェーダーファイルの内容を文字列として読み込み
	std::string vsFileData{
		std::istreambuf_iterator<char>(vsStream),
		std::istreambuf_iterator<char>()
	};
	auto vsSourceCode = vsFileData.c_str();
	auto vsSourceCodeSize = vsFileData.length();

	// シェーダーのコンパイルと頂点シグネチャの生成
	auto vs = device.CompileShader({ vsFilePath, vsSourceCode, vsSourceCodeSize, "VSMain", ShaderType::VertexShader });
	auto vsSig = device.CreateVertexShaderSignature({ vs });

	// デフォルトのピクセルシェーダ読み込み
	constexpr char psFilePath[] = "SourceFiles/DX3D/Assets/Shaders/Pixel/PS_Default.hlsl";
	std::ifstream psStream(psFilePath);
	if (!psStream) {
		DX3DLogThrowError("PSファイルを開くのに失敗");
	}
	// シェーダーファイルの内容を文字列として読み込み
	std::string psFileData{
		std::istreambuf_iterator<char>(psStream),
		std::istreambuf_iterator<char>()
	};
	auto psSourceCode = psFileData.c_str();
	auto psSourceCodeSize = psFileData.length();

	auto ps = device.CompileShader({ psFilePath, psSourceCode, psSourceCodeSize, "PSMain", ShaderType::PixelShader });

	// グラフィックスパイプラインステートの生成
	pipeline_ = device.CreateGraphicsPipelineState({ *vsSig, *ps });

	// ラスタライザーステートの生成
	rasterizer_ = device.CreateRasterizerState({
		.fillMode = FillMode::Solid,
		.cullMode = CullMode::Back,
		});


}

dx3d::GraphicsEngine::~GraphicsEngine()
{
}

GraphicsDevice& dx3d::GraphicsEngine::GetGraphicsDevice() noexcept
{
	return *graphics_device_;
}

DeviceContext& dx3d::GraphicsEngine::GetDeviceContext() noexcept
{
	return *device_context_;
}

void dx3d::GraphicsEngine::SetSwapChain(SwapChain& _swapChain)
{
	swap_chain_ = &_swapChain;
}

void dx3d::GraphicsEngine::BeginFrame()
{
	auto& context = *device_context_;
	context.ClearAndSetBackBuffer(*swap_chain_, { 0.27f, 0.39f, 0.55f, 1.0f });	// 初期色でクリア

}

void dx3d::GraphicsEngine::Render(VertexBuffer& _vb, IndexBuffer& _ib)
{
	device_context_->SetGraphicsPipelineState(*pipeline_);
	device_context_->SetRasterizerState(*rasterizer_);
	device_context_->SetViewportSize(swap_chain_->GetSize());

	device_context_->SetVertexBuffer(_vb);
	device_context_->SetIndexBuffer(_ib);
	device_context_->DrawIndexed(_ib.GetIndexCount(), 0, 0);
}

void dx3d::GraphicsEngine::RenderInstanced(VertexBuffer& _vb, IndexBuffer& _ib, VertexBuffer& _instanceVB, ui32 _instanceCount, ui32 _startInstance)
{
	device_context_->SetGraphicsPipelineState(*pipeline_);
	device_context_->SetRasterizerState(*rasterizer_);
	device_context_->SetViewportSize(swap_chain_->GetSize());

	device_context_->SetVertexBuffers(_vb, _instanceVB);
	device_context_->SetIndexBuffer(_ib);
	device_context_->DrawIndexedInstanced(_ib.GetIndexCount(), _instanceCount, 0, 0, _startInstance);
}

void dx3d::GraphicsEngine::EndFrame()
{
	auto& context = *device_context_;
	auto& device = *graphics_device_;
	device.ExecuteCommandList(context);

	// 画面に表示
	swap_chain_->Present();
}
