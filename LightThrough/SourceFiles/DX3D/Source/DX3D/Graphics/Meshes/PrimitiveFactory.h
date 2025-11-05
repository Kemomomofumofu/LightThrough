#pragma once
/**
 * @file PrimitiveFactory.h
 * @brief 図形を生成する
 * @author Arima Keita
 * @date 2025-09-12
 */

 /*---------- インクルード ----------*/

namespace dx3d {
	class GraphicsDevice;
	struct MeshHandle;
	class MeshRegistry;

	namespace PrimitiveFactory {
		MeshHandle CreateCube(GraphicsDevice& _device, MeshRegistry& _registry);
		MeshHandle CreateQuad(GraphicsDevice& _device, MeshRegistry& _registry);
		MeshHandle CreateSphere(GraphicsDevice& _device, MeshRegistry& _registry, uint32_t _slices = 10, uint32_t _stacks = 10);
		//ecs::Mesh CreateLine(GraphicsDevice& _device, DirectX::XMFLOAT3 _start, DirectX::XMFLOAT3 _end);

	}	// namespace PrimitiveFactory
}	// namespace dx3d