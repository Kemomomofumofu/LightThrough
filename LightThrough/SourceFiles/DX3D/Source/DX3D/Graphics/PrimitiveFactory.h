#pragma once
/**
 * @file PrimitiveFactory.h
 * @brief 図形を生成する
 * @author Arima Keita
 * @date 2025-09-12
 */

 /*---------- インクルード ----------*/
#include <DirectXMath.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <Game/Components/Mesh.h>

namespace ecs {
	struct Mesh;
}

namespace dx3d {
	namespace PrimitiveFactory {
		ecs::Mesh CreateCube(GraphicsDevice& _device, f32 _size = 1.0f);
		ecs::Mesh CreateQuad(GraphicsDevice& _device, f32 _size = 1.0f);
		ecs::Mesh CreateSphere(GraphicsDevice& _device, ui32 _slices = 10, ui32 _stacks = 10, f32 _radius = 0.5f);
		//ecs::Mesh CreateLine(GraphicsDevice& _device, DirectX::XMFLOAT3 _start, DirectX::XMFLOAT3 _end);

	}
}