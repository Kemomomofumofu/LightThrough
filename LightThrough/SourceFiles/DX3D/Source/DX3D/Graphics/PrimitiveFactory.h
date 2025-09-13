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
	class Mesh;
}

namespace dx3d {
	namespace PrimitiveFactory {
		ecs::Mesh CreateCube(GraphicsDevice& _device, float _size = 1.0f);
		//ecs::Mesh CreateQuad(GraphicsDevice& _device, float _size = 1.0f);
		//ecs::Mesh CreateSphere(GraphicsDevice& _device, int _slices, int _stacks, float _radius);
		//ecs::Mesh CreateLine(GraphicsDevice& _device, DirectX::XMFLOAT3 _start, DirectX::XMFLOAT3 _end);

	}
}