#pragma once
/**
 * @file GraphicsEngine.h
 * @brief グラフィックエンジンクラス
 * @author Arima Keita
 * @date 2025-07-11
 */

 /*---------- インクルード ----------*/
#include <vector>
#include <wrl/client.h>
#include <DX3D/Core/Core.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Graphics/ShaderCache.h>
#include <DX3D/Graphics/SwapChain.h>
#include <DX3D/Graphics/PipelineCache.h>
#include <DX3D/Graphics/PipelineKey.h>
#include <DX3D/Graphics/Meshes/MeshRegistry.h>

namespace dx3d {
	class TextureRegistry;

	struct GraphicsEngineDesc {
		BaseDesc base;
	};

	class GraphicsEngine final : public Base {
	public:
		explicit GraphicsEngine(const GraphicsEngineDesc& _desc);
		virtual ~GraphicsEngine() override;

		GraphicsDevice& GetGraphicsDevice() noexcept;
		DeviceContext& GetDeferredContext() noexcept;
		ID3D11DeviceContext* GetImmediateContext() noexcept;

		MeshRegistry& GetMeshRegistry() noexcept;
		TextureRegistry& GetTextureRegistry() noexcept { return *texture_registry_; }
		ShaderCache& GetShaderCache() noexcept { return *shader_cache_; }
		const Rect& GetScreenSize() { return swap_chain_->GetSize(); }

		void SetSwapChain(SwapChain& _swapChain);

		void BeginFrame();
		void Render(VertexBuffer& _vb, IndexBuffer& _ib, PipelineKey _key = { VertexShaderKind::Default, PixelShaderKind::Default });
		void RenderInstanced(VertexBuffer& _vb, IndexBuffer& _ib, VertexBuffer& _instanceVB, uint32_t _instanceCount, uint32_t _startInstance = 0, PipelineKey _key = { VertexShaderKind::Instanced, PixelShaderKind::Default });
		void RenderInstancedOnImmediate(VertexBuffer& _vb, IndexBuffer& _ib, VertexBuffer& _instanceVB, uint32_t _instanceCount, uint32_t _startInstance = 0, PipelineKey _key = { VertexShaderKind::Instanced, PixelShaderKind::Default });
		void EndFrame();

		ID3D11RenderTargetView* GetSceneColorRTV() const noexcept { return scene_color_.rtv.Get(); }
		ID3D11ShaderResourceView* GetSceneColorSRV() const noexcept { return scene_color_.srv.Get(); }
		ID3D11DepthStencilView* GetSceneDepthDSV() const noexcept { return scene_depth_dsv_.Get(); }
		ID3D11ShaderResourceView* GetSceneDepthSRV() const noexcept { return scene_depth_srv_.Get(); }

	private:
		struct OffscreenTarget {
			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture{};
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv{};
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv{};
		};

		enum class PostPassKind {
			Outline,
			Vhs,
		};

		void EnsureFrameResources();
		void CreateOffscreenTarget(uint32_t _width, uint32_t _height, DXGI_FORMAT _format, OffscreenTarget& _outTarget);

		void ResolveSceneToBackBuffer();

		// ---------- ポストチェーン ----------
		void InitPostProcessChain();
		void RunPostProcessChain();
		void ExecutePostPass(PostPassKind _kind, const OffscreenTarget& _src, OffscreenTarget& _dst);
		void ExecutePassthrough(const OffscreenTarget& _src, OffscreenTarget& _dst);

	private:
		std::shared_ptr<GraphicsDevice> graphics_device_{};
		DeviceContextPtr deferred_context_{};
		GraphicsPipelineStatePtr pipeline_{};
		InputLayoutPtr vs_layout_{};
		std::unique_ptr<ShaderCache> shader_cache_{};
		std::unique_ptr<PipelineCache> pipeline_cache_{};
		SwapChain* swap_chain_{};
		std::unique_ptr<MeshRegistry> mesh_registry_{};
		std::unique_ptr<TextureRegistry> texture_registry_{};

		Rect frame_resource_size_{};
		OffscreenTarget scene_color_{};
		OffscreenTarget post_a_{};
		OffscreenTarget post_b_{};
		Microsoft::WRL::ComPtr<ID3D11Texture2D> scene_depth_tex_{};
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> scene_depth_dsv_{};
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scene_depth_srv_{};
		Microsoft::WRL::ComPtr<ID3D11SamplerState> post_linear_sampler_{};

		std::vector<PostPassKind> post_chain_{};
	};
}
