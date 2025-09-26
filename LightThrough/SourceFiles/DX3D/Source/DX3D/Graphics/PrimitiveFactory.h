#pragma once
/**
 * @file PrimitiveFactory.h
 * @brief 図形を生成する
 * @author Arima Keita
 * @date 2025-09-12
 */

 /*---------- インクルード ----------*/
#include <unordered_map>
#include <DirectXMath.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <Game/Components/Mesh.h>

namespace ecs {
	struct Mesh;
}

namespace dx3d {
	namespace PrimitiveFactory {
		struct CachedMesh {
			std::shared_ptr<VertexBuffer> vb;
			std::shared_ptr<IndexBuffer> ib;
			ui32 indexCount{};
			bool Valid() const noexcept { return vb && ib && indexCount > 0; }
		};

		namespace {
			CachedMesh g_cube;
			CachedMesh g_quad;

			struct SphereKey {
				ui32 slices{};
				ui32 stacks{};
				bool operator== (const SphereKey& o) const noexcept {
					return slices == o.slices && stacks == o.stacks;
				}
			};

			struct SphereKeyHash {
				size_t operator()(const SphereKey& k)const noexcept {
					return (static_cast<size_t>(k.slices) << 32) ^ static_cast<size_t>(k.stacks);
				}
			};

			std::unordered_map<SphereKey, CachedMesh, SphereKeyHash> g_spheres;
		}

		constexpr DirectX::XMFLOAT4 DEFAULT_COLOR{ 1.0f, 1.0f, 1.0f, 0.75f };
		

		ecs::Mesh CreateCube(GraphicsDevice& _device);
		ecs::Mesh CreateQuad(GraphicsDevice& _device);
		ecs::Mesh CreateSphere(GraphicsDevice& _device, ui32 _slices = 10, ui32 _stacks = 10);
		//ecs::Mesh CreateLine(GraphicsDevice& _device, DirectX::XMFLOAT3 _start, DirectX::XMFLOAT3 _end);

	}
}