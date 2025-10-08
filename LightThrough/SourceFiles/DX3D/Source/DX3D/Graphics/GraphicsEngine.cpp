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

/*---------- 名前空間 ----------*/
using namespace dx3d;

dx3d::GraphicsEngine::GraphicsEngine(const GraphicsEngineDesc& _desc)
	: Base(_desc.base)
{
	graphics_device_ = std::make_shared<GraphicsDevice>(GraphicsDeviceDesc{ logger_ });

	auto& device = *graphics_device_;
	device_context_ = device.CreateDeviceContext();

	// ラスタライザーステートの生成
	rasterizer_ = device.CreateRasterizerState({
		.fillMode = FillMode::Solid,
		.cullMode = CullMode::Back,
		});


	// パイプラインキャッシュの生成
	pipeline_cache_ = device.CreatePipelineCache({});

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
	PipelineKey key{
		VertexShaderKind::Default,
		PixelShaderKind::Default,
	};
	auto pso = pipeline_cache_->GetOrCreate(key);

	device_context_->SetGraphicsPipelineState(*pso);
	device_context_->SetRasterizerState(*rasterizer_);	// [ToDo] pso で設定できるようにしたいね
	device_context_->SetViewportSize(swap_chain_->GetSize());

	device_context_->SetVertexBuffer(_vb);
	device_context_->SetIndexBuffer(_ib);
	device_context_->DrawIndexed(_ib.GetIndexCount(), 0, 0);
}

void dx3d::GraphicsEngine::RenderInstanced(VertexBuffer& _vb, IndexBuffer& _ib, VertexBuffer& _instanceVB, uint32_t _instanceCount, uint32_t _startInstance)
{
	PipelineKey key{
		VertexShaderKind::Instanced,
		PixelShaderKind::Default,
	};
	PromoteInstancing(key, _instanceCount);	// インスタンス数に応じてパイプラインキーを更新
	auto pso = pipeline_cache_->GetOrCreate(key);

	device_context_->SetGraphicsPipelineState(*pso);
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
