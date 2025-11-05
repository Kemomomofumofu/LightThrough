/**
 * @file SwapChain.cpp
 * @brief スワップチェイン
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DX3D/Graphics/SwapChain.h>


/**
 * @brief コンストラクタ
 * @param _desc スワップチェインの設定
 * @param _gDesc グラフィックリソースの設定
 */
dx3d::SwapChain::SwapChain(const SwapChainDesc& _desc, const GraphicsResourceDesc& _gDesc)
	: GraphicsResource(_gDesc),
	size_(_desc.winSize)
{
	if (!_desc.winHandle) {
		DX3DLogThrowInvalidArg("WindowHandleがありません");
	}

	DXGI_SWAP_CHAIN_DESC dxgiDesc{};

	dxgiDesc.BufferDesc.Width = max(1, _desc.winSize.width);
	dxgiDesc.BufferDesc.Height = max(1, _desc.winSize.height);
	dxgiDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiDesc.BufferCount = 2;
	dxgiDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	dxgiDesc.OutputWindow = static_cast<HWND>(_desc.winHandle);
	dxgiDesc.SampleDesc.Count = 1;
	dxgiDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiDesc.Windowed = TRUE;

	DX3DGraphicsLogThrowOnFail(factory_.CreateSwapChain(&device_, &dxgiDesc, &swap_chain_), "CreateSwapChain に 失敗");

	ReloadBuffers();
}


dx3d::Rect dx3d::SwapChain::GetSize() const noexcept
{
	return size_;
}


/**
 * @brief 描画、切り替え
 * @param _vsync フラグ
 */
void dx3d::SwapChain::Present(bool _vsync)
{
	DX3DGraphicsLogThrowOnFail(swap_chain_->Present(_vsync, 0), "Present に 失敗");
}

/**
 * @brief バッファのロード
 */
void dx3d::SwapChain::ReloadBuffers()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> buffer{};
	DX3DGraphicsLogThrowOnFail(swap_chain_->GetBuffer(0, IID_PPV_ARGS(&buffer)), "GetBuffer に 失敗");
	DX3DGraphicsLogThrowOnFail(device_.CreateRenderTargetView(buffer.Get(), nullptr, &rtv_), "CreateRenderTargetView に 失敗");

	// todo: 仮
	depth_tex_.Reset();
	dsv_.Reset();

	// 深度テクスチャ作成
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = (std::max)(1, size_.width);
	depthDesc.Height = (std::max)(1, size_.height);
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depth{};
	DX3DGraphicsLogThrowOnFail(device_.CreateTexture2D(&depthDesc, nullptr, &depth), "CreateTexture2D に 失敗");
	DX3DGraphicsLogThrowOnFail(device_.CreateDepthStencilView(depth.Get(), nullptr, &dsv_), "CreateDepthStencilView に 失敗");

	depth_tex_ = std::move(depth);

}
