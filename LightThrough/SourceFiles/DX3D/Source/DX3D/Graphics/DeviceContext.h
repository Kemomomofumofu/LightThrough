#pragma once
/**
 * @file DeviceContext.h
 * @brief デバイスコンテキスト
 * @author Arima Keita
 * @date 2025-06-25
 */

 /*---------- インクルード ----------*/
#include <DirectXMath.h>
#include <DX3D/Graphics/GraphicsResource.h>


/**
 * @brief デバイスコンテキスト
 *
 * デバイスコンテキストの仕事をまとめたクラス
 */
namespace dx3d {
	class DeviceContext final : public GraphicsResource
	{
	public:
		/**
		 * @brief コンストラクタ
		 * @param _gDesc グラフィックスリソースの設定
		 */
		explicit DeviceContext(const GraphicsResourceDesc& _gDesc);

		/**
		 * @brief デバイスコンテキストを取得
		 * @return ID3D11DeviceContextのComPtr
		 */
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetDeferredContext() const noexcept;

		/**
		 * @brief バックバッファをクリアしてセット
		 * @param _swapChain スワップチェイン
		 * @param _color クリアする色
		 */
		void ClearAndSetBackBuffer(const SwapChain& _swapChain, const DirectX::XMFLOAT4& _color);

		/**
		 * @brief グラフィックスパイプラインステートをセット
		 * @param _pipepline パイプラインステート
		 */
		void SetGraphicsPipelineState(const GraphicsPipelineState& _pipepline);

		/**
		 * @brief 頂点バッファをセット
		 * @param _buffer 頂点バッファ
		 */
		void SetVertexBuffer(const VertexBuffer& _buffer);

		/**
		 * @brief 複数の頂点バッファをセット
		 * @param _startSlot 開始スロット
		 * @param _buffers 頂点バッファの配列
		 * @param _count バッファの数
		 */
		void SetVertexBuffers(uint32_t _startSlot, const VertexBuffer* const* _buffers, uint32_t _count);

		/**
		 * @brief 2つの頂点バッファをセット
		 * @param _vb0 1つ目の頂点バッファ
		 * @param _vb1 2つ目の頂点バッファ
		 */
		void SetVertexBuffers(const VertexBuffer& _vb0, const VertexBuffer& _vb1);

		/**
		 * @brief 入力レイアウトをセット
		 * @param _layout 入力レイアウト
		 */
		void SetInputLayout(const InputLayout& _layout);

		/**
		 * @brief インデックスバッファをセット
		 * @param _buffer インデックスバッファ
		 */
		void SetIndexBuffer(const IndexBuffer& _buffer);

		/**
		 * @brief 頂点シェーダに定数バッファをセット
		 * @param _slot スロット番号
		 * @param _buffer 定数バッファ
		 */
		void VSSetConstantBuffer(uint32_t _slot, const ConstantBuffer& _buffer);

		/**
		 * @brief ピクセルシェーダに定数バッファをセット
		 * @param _slot スロット番号
		 * @param _buffer 定数バッファ
		 */
		void PSSetConstantBuffer(uint32_t _slot, const ConstantBuffer& _buffer);
		/**
		 * @brief ピクセルシェーダのシェーダーリソースビューをセットする
		 * @param _slot 開始スロット
		 * @param _numResources リソース数
		 * @param _srv シェーダーリソースビュー
		 */
		void PSSetShaderResources(uint32_t _startSlot, uint32_t _numResources, ID3D11ShaderResourceView* const* _ppSrv);

		/**
		 * @brief ピクセルシェーダのサンプラーをセットする
		 * @param _slot 開始スロット
		 * @param _numSamplers サンプラー数
		 * @param _sampler サンプラー
		 */
		void PSSetSamplers(uint32_t _startSlot, uint32_t _numSamplers, ID3D11SamplerState* const* _ppSampler);

		/**
		 * @brief コンピュートシェーダをセット
		 * @param _cs コンピュートシェーダー
		 */
		void CSSetShader(ID3D11ComputeShader* _cs);

		/**
		 * @brief コンピュートシェーダの定数バッファをセット
		 * @param _slot スロット番号
		 * @param _cb 定数バッファ
		 */
		void CSSetConstantBuffer(uint32_t _slot, const ConstantBuffer& _cb);

		/**
		 * @brief コンピュートシェーダの定数バッファを複数セット
		 * @param _startSlot 開始スロット
		 * @param _numBuffers バッファ数
		 * @param _buffers バッファの配列
		 */
		void CSSetConstantBuffers(uint32_t _startSlot, uint32_t _numBuffers, const ConstantBuffer* const* _buffers);

		/**
		 * @brief コンピュートシェーダのシェーダーリソースビューをセット
		 * @param _startSlot 開始スロット
		 * @param _buffer 構造化バッファ
		 */
		void CSSetShaderResources(uint32_t _startSlot, StructuredBuffer& _buffer);
		/**
		 * @brief コンピュートシェーダのシェーダーリソースビューをセット
		 * @param _startSlot 開始スロット
		 * @param _srv シェーダーリソースビュー
		 */
		void CSSetShaderResources(uint32_t _startSlot, ID3D11ShaderResourceView* _srv);
		/**
		 * @brief コンピュートシェーダのシェーダーリソースビューをセット
		 * @param _startSlot 開始スロット
		 * @param _numViews ビュー数
		 * @param _srvs SRVの配列
		 */
		void CSSetShaderResources(uint32_t _startSlot, uint32_t _numViews, ID3D11ShaderResourceView* const* _srvs);

		/**
		 * @brief コンピュートシェーダーのアンオーダードアクセスビューをセット
		 * @param _slot スロット番号
		 * @param _buffer 構造化バッファ
		 */
		void CSSetUnorderedAccessView(uint32_t _slot, RWStructuredBuffer* _buffer);

		/**
		 * @brief コンピュートシェーダのリソースをクリア
		 * @param _numSRVs クリアするSRVの数
		 * @param _numUAVs クリアするUAVの数
		 */
		void CSClearResources(uint32_t _numSRVs, uint32_t _numUAVs);

		/**
		 * @brief コンピュートシェーダーディスパッチ
		 * @param _x スレッドグループ数X
		 * @param _y スレッドグループ数Y
		 * @param _z スレッドグループ数Z
		 */
		void Dispatch(uint32_t _x, uint32_t _y, uint32_t _z);

		/**
		 * @brief リソースコピー
		 * @param _dst コピー先ステージングバッファ
		 * @param _src コピー元構造化バッファ
		 */
		void CopyResource(StagingBuffer& _dst, RWStructuredBuffer& _src);

		/**
		 * @brief 定数バッファをセット
		 * @param _slot スロット番号
		 * @param _buffer 定数バッファ
		 */
		void SetConstantBuffer(uint32_t _slot, const ConstantBuffer& _buffer);

		/**
		 * @brief ビューポートサイズをセット
		 * @param _size ビューポートのサイズ
		 */
		void SetViewportSize(const Rect& _size);

		/**
		 * @brief 三角形リストを描画
		 * @param _vertexCount 頂点数
		 * @param _startVertexLocation 開始頂点位置
		 */
		void DrawTriangleList(uint32_t _vertexCount, uint32_t _startVertexLocation);

		/**
		 * @brief インデックス付き描画
		 * @param _indexCount インデックス数
		 * @param _startIndex 開始インデックス
		 * @param _baseVertex ベース頂点
		 */
		void DrawIndexed(uint32_t _indexCount, uint32_t _startIndex, uint32_t _baseVertex);

		/**
		 * @brief インスタンス描画
		 * @param _indexCount インデックス数
		 * @param _instanceCount インスタンス数
		 * @param _startIndex 開始インデックス
		 * @param _baseVertex ベース頂点
		 * @param _startInstance 開始インスタンス
		 */
		void DrawIndexedInstanced(uint32_t _indexCount, uint32_t _instanceCount, uint32_t _startIndex, uint32_t _baseVertex, uint32_t _startInstance);


		void* ReadbackBuffer(StagingBuffer& _dst, RWStructuredBuffer& _src);
		// void UnmapReadback(StagingBuffer& _buffer);
	private:
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> deferred_context_{};

		friend class GraphicsDevice;
	};
}
