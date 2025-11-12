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
#include <DX3D/Graphics/VertexShaderSignature.h>
#include <DX3D/Graphics/Buffers/VertexBuffer.h>
#include <DX3D/Graphics/Buffers/IndexBuffer.h>
#include <DX3D/Graphics/Buffers/ConstantBuffer.h>

/*---------- 名前空間 ----------*/

namespace dx3d {
	/**
	 * @brief コンストラクタ
	 * @param _desc グラフィックデバイスの定義
	 */
	GraphicsDevice::GraphicsDevice(const GraphicsDeviceDesc& _desc) : Base(_desc.base)
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

	GraphicsDevice::~GraphicsDevice()
	{
	}



	/**
	 * @brief スワップチェーンを生成する
	 * @param _desc スワップチェーンの定義
	 * @return 戻り値の説明スワップチェーンのSharedPtr
	 */
	SwapChainPtr GraphicsDevice::CreateSwapChain(const SwapChainDesc& _desc)
	{
		return std::make_shared<SwapChain>(_desc, GetGraphicsResourceDesc());
	}

	/**
	 * @brief デバイスコンテキストを生成
	 * @return デバイスコンテキストのSharedPtr
	 */
	DeviceContextPtr GraphicsDevice::CreateDeviceContext()
	{
		return std::make_shared<DeviceContext>(GetGraphicsResourceDesc());
	}

	/**
	 * @brief シェーダをコンパイル
	 * @param _desc シェーダの定義
	 * @return コンパイルされたシェーダのSharedPtr
	 */
	ShaderBinaryPtr GraphicsDevice::CompileShader(const ShaderBinary::ShaderCompileDesc& _desc) const
	{
		return std::make_shared<ShaderBinary>(_desc, GetGraphicsResourceDesc());
	}

	/**
	 * @brief
	 * @param _desc
	 * @return PipelineStateのSharedPtr
	 */
	GraphicsPipelineStatePtr GraphicsDevice::CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& _desc) const
	{
		return std::make_shared<GraphicsPipelineState>(_desc, GetGraphicsResourceDesc());
	}

	RasterizerStatePtr GraphicsDevice::CreateRasterizerState(const RasterizerStateDesc& _desc)
	{
		return std::make_shared<RasterizerState>(_desc, GetGraphicsResourceDesc());
	}

	/**
	 * @brief VertexBufferを生成する
	 * @param _desc VertexBufferの定義
	 * @return VertexBufferのSharedPtr
	 */
	VertexBufferPtr GraphicsDevice::CreateVertexBuffer(const VertexBufferDesc& _desc)
	{
		return std::make_shared<VertexBuffer>(_desc, GetGraphicsResourceDesc());
	}

	/**
	 * @brief IndexBufferを生成する
	 * @param _desc IndexBufferの定義
	 * @return IndexBufferのSharedPtr
	 */
	IndexBufferPtr GraphicsDevice::CreateIndexBuffer(const IndexBufferDesc& _desc)
	{
		return std::make_shared<IndexBuffer>(_desc, GetGraphicsResourceDesc());
	}

	/**
	 * @brief ConstantBuffer生成する
	 * @param _desc ConstantBufferの定義
	 * @return ConstantBufferのSharedPtr
	 */
	ConstantBufferPtr GraphicsDevice::CreateConstantBuffer(const ConstantBufferDesc& _desc)
	{
		return std::make_shared<ConstantBuffer>(_desc, GetGraphicsResourceDesc());
	}

	/**
	 * @brief Instancingで使うバッファの生成
	 * @param _data インスタンスのデータ群
	 * @return 頂点バッファとしてまとめられたインスタンスバッファ
	 */
	auto GraphicsDevice::CreateInstanceBuffer(const std::vector<InstanceData>& _data)
	{
		VertexBufferDesc desc{
			.vertexList = _data.data(),
			.vertexListSize = static_cast<uint32_t>(_data.size() * sizeof(InstanceData)),
			.vertexSize = sizeof(InstanceData),
		};

		return CreateVertexBuffer(desc);
	}

	std::unique_ptr<PipelineCache> GraphicsDevice::CreatePipelineCache(const PipelineCache::PipelineCacheDesc& _desc)
	{
		return std::make_unique<PipelineCache>(_desc, GetGraphicsResourceDesc());
	}

	HRESULT GraphicsDevice::CreateTexture2D(const D3D11_TEXTURE2D_DESC* _desc, const D3D11_SUBRESOURCE_DATA* _initialData, ID3D11Texture2D** _texture) const noexcept
	{
		return d3d_device_->CreateTexture2D(_desc, _initialData, _texture);
	}

	HRESULT GraphicsDevice::CreateDepthStencilView(ID3D11Resource* _resource, const D3D11_DEPTH_STENCIL_VIEW_DESC* _desc, ID3D11DepthStencilView** _dvs) const noexcept
	{
		return d3d_device_->CreateDepthStencilView(_resource, _desc, _dvs);
	}

	HRESULT GraphicsDevice::CreateShaderResourceView(ID3D11Resource* _resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* _desc, ID3D11ShaderResourceView** _srv) const noexcept
	{
		return d3d_device_->CreateShaderResourceView(_resource, _desc, _srv);
	}


	VertexShaderSignaturePtr GraphicsDevice::CreateVertexShaderSignature(const VertexShaderSignatureDesc& _desc) const
	{
		return std::make_shared<VertexShaderSignature>(_desc, GetGraphicsResourceDesc());
	}

	InputLayoutPtr GraphicsDevice::CreateInputLayout(const InputLayoutDesc& _desc) const
	{
		return std::make_shared<InputLayout>(_desc, GetGraphicsResourceDesc());
	}

	/**
	 * @brief コマンドの実行
	 * @param _context への参照
	 */
	void GraphicsDevice::ExecuteCommandList(DeviceContext& _context)
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
	GraphicsResourceDesc GraphicsDevice::GetGraphicsResourceDesc() const noexcept
	{
		return { {logger_}, shared_from_this(), *d3d_device_.Get(), *dxgi_factory_.Get() };
	}

}