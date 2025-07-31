#pragma once
/**
 * @file GraphicsResource.h
 * @brief グラフィックのリソース
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Core/Common.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Graphics/GraphicsLogUtils.h>

#include <d3d11.h>
#include <wrl.h>


/**
 * @brief グラフィックスリソース
 *
 * テクスチャやバッファなど、リソース生成・管理に必要な情報を管理するクラス。
 */
namespace dx3d {
	struct GraphicsResourceDesc {
		BaseDesc base;
		std::shared_ptr<const GraphicsDevice> graphicsDevice;
		ID3D11Device& device;
		IDXGIFactory& factory;
	};

	class GraphicsResource : public Base {
	public:
		explicit GraphicsResource(const GraphicsResourceDesc& _desc) :
			Base(_desc.base),
			graphics_device_(_desc.graphicsDevice),
			device_(_desc.device),
			factory_(_desc.factory)
		{
		}

	protected:
		std::shared_ptr<const GraphicsDevice> graphics_device_;
		ID3D11Device& device_;
		IDXGIFactory& factory_;
	};
}