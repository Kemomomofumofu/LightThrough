/**
 * @file GraphicsDevice.cpp
 * @brief グラフィックデバイスの定義
 * @author Arima Keita
 * @date 2025-07-16
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/GraphicsLogUtils.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/DeviceContext.h>
#include <DX3D/Graphics/ShaderBinary.h>
#include <DX3D/Graphics/GraphicsPipelineState.h>
#include <DX3D/Graphics/RasterizerState.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/VertexShaderSignature.h>
#include <DX3D/Graphics/IndexBuffer.h>
#include <DX3D/Graphics/ConstantBuffer.h>

/*---------- 名前空間 ----------*/
using namespace dx3d;


/**
 * @brief コンストラクタ
 * @param _desc グラフィックデバイスの定義
 */
dx3d::GraphicsDevice::GraphicsDevice(const GraphicsDeviceDesc& _desc): Base(_desc.base)
{
	D3D_FEATURE_LEVEL featureLevel{};
	UINT createDeviceFlags{};

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// デバイスの生成
	DX3DGraphicsLogThrowOnFail(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, NULL, 0, D3D11_SDK_VERSION, &d3d_device_, &featureLevel, &d3d_context_), "Direct3D11の初期化に失敗");
	// DXGIデバイスを取得
	DX3DGraphicsLogThrowOnFail(d3d_device_->QueryInterface(IID_PPV_ARGS(&dxgi_device_)), "IDXGIDeviceの取得に失敗");
	// 使用中のGPUアダプタを取得
	DX3DGraphicsLogThrowOnFail(dxgi_device_->GetParent(IID_PPV_ARGS(&dxgi_adapter_)), "IDXGIAdapterの取得に失敗");
	// DXGIファクトリを取得
	DX3DGraphicsLogThrowOnFail(dxgi_adapter_->GetParent(IID_PPV_ARGS(&dxgi_factory_)), "IDXGIFactoryの取得に失敗");
}

dx3d::GraphicsDevice::~GraphicsDevice()
{
}



/**
 * @brief スワップチェーンを生成する
 * @param _desc スワップチェーンの定義
 * @return 戻り値の説明スワップチェーンのSharedPtr
 */
SwapChainPtr dx3d::GraphicsDevice::CreateSwapChain(const SwapChainDesc& _desc)
{
	return std::make_shared<SwapChain>(_desc, GetGraphicsResourceDesc());
}

/**
 * @brief デバイスコンテキストを生成
 * @return デバイスコンテキストのSharedPtr
 */
DeviceContextPtr dx3d::GraphicsDevice::CreateDeviceContext()
{
	return std::make_shared<DeviceContext>(GetGraphicsResourceDesc());
}

/**
 * @brief シェーダをコンパイル
 * @param _desc シェーダの定義
 * @return コンパイルされたシェーダのSharedPtr
 */
ShaderBinaryPtr dx3d::GraphicsDevice::CompileShader(const ShaderCompileDesc& _desc)
{
	return std::make_shared<ShaderBinary>(_desc, GetGraphicsResourceDesc());
}

/**
 * @brief 
 * @param _desc 
 * @return PipelineStateのSharedPtr
 */
GraphicsPipelineStatePtr dx3d::GraphicsDevice::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& _desc)
{
	return std::make_shared<GraphicsPipelineState>(_desc, GetGraphicsResourceDesc());
}

RasterizerStatePtr dx3d::GraphicsDevice::CreateRasterizerState(const RasterizerStateDesc& _desc)
{
	return std::make_shared<RasterizerState>(_desc, GetGraphicsResourceDesc());
}

/**
 * @brief VertexBufferを生成する
 * @param _desc VertexBufferの定義
 * @return VertexBufferのSharedPtr
 */
VertexBufferPtr dx3d::GraphicsDevice::CreateVertexBuffer(const VertexBufferDesc& _desc)
{
	return std::make_shared<VertexBuffer>(_desc, GetGraphicsResourceDesc());
}

/**
 * @brief IndexBufferを生成する
 * @param _desc IndexBufferの定義
 * @return IndexBufferのSharedPtr
 */
IndexBufferPtr dx3d::GraphicsDevice::CreateIndexBuffer(const IndexBufferDesc& _desc)
{
	return std::make_shared<IndexBuffer>(_desc, GetGraphicsResourceDesc());
}

/**
 * @brief ConstantBuffer生成する
 * @param _desc ConstantBufferの定義
 * @return ConstantBufferのSharedPtr
 */
ConstantBufferPtr dx3d::GraphicsDevice::CreateConstantBuffer(const ConstantBufferDesc& _desc)
{
	return std::make_shared<ConstantBuffer>(_desc, GetGraphicsResourceDesc());
}

VertexShaderSignaturePtr dx3d::GraphicsDevice::CreateVertexShaderSignature(const VertexShaderSignatureDesc& _desc)
{
	return std::make_shared<VertexShaderSignature>(_desc, GetGraphicsResourceDesc());
}


/**
 * @brief コマンドの実行
 * @param _context への参照
 */
void dx3d::GraphicsDevice::ExecuteCommandList(DeviceContext& _context)
{
	Microsoft::WRL::ComPtr<ID3D11CommandList> list{};
	// コマンドをリストにまとめて取得
	DX3DGraphicsLogThrowOnFail(_context.context_->FinishCommandList(false, &list), "FinishCommandListが失敗");
	// 取得したコマンドリストを実行
	d3d_context_->ExecuteCommandList(list.Get(), false);
}


/**
 * @brief グラフィックリソースの取得
 * @return GraphicsResourceDesc構造体
 */
GraphicsResourceDesc dx3d::GraphicsDevice::GetGraphicsResourceDesc() const noexcept
{
	return { {logger_}, shared_from_this(), *d3d_device_.Get(), *dxgi_factory_.Get()};
}
