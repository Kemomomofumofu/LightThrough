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

	DX3DGraphicsLogThrowOnFail(factory_.CreateSwapChain(&device_, &dxgiDesc, &swap_chain_), "CreateSwapChainが失敗");

	ReloadBuffers();
}


dx3d::Rect dx3d::SwapChain::GetSize() const noexcept{
	return size_;
}


/**
 * @brief 描画、切り替え
 * @param _vsync フラグ
 */
void dx3d::SwapChain::Present(bool _vsync){
	DX3DGraphicsLogThrowOnFail(swap_chain_->Present(_vsync, 0), "Present を 失敗");
}


/**
 * @brief バッファのロード
 */
void dx3d::SwapChain::ReloadBuffers(){
	Microsoft::WRL::ComPtr<ID3D11Texture2D> buffer{};
	DX3DGraphicsLogThrowOnFail(swap_chain_->GetBuffer(0, IID_PPV_ARGS(&buffer)), "GetBuffer を 失敗");
	DX3DGraphicsLogThrowOnFail(device_.CreateRenderTargetView(buffer.Get(), nullptr, &rtv_), "CreateRenderTargetView を 失敗");


}
