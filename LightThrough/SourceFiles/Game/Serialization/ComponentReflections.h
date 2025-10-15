#pragma once
/**
 * @file ComponentReflections.h
 * @brief コンポーネントのリフレクションを登録するヘッダファイル
 * @author Arima Keita
 * @date 2025-10-06
 */

 /*---------- インクルード ----------*/
#include <Game/Serialization/ComponentReflection.h>

#include <Game/Components/Transform.h>
#include <Game/Components/Camera.h>
#include <Game/Components/CameraController.h>
#include <Game/Components/MeshRenderer.h>
#include <Game/Components/Collider.h>


// Transform
ECS_REFLECT_BEGIN(ecs::Transform)
ECS_REFLECT_FIELD(position),
ECS_REFLECT_FIELD(rotationQuat),
ECS_REFLECT_FIELD(scale)
ECS_REFLECT_END()

// Camera
ECS_REFLECT_BEGIN(ecs::Camera)
ECS_REFLECT_FIELD(fovY),
ECS_REFLECT_FIELD(aspectRatio),
ECS_REFLECT_FIELD(nearZ),
ECS_REFLECT_FIELD(farZ),
ECS_REFLECT_FIELD(isMain),
ECS_REFLECT_FIELD(isActive)
ECS_REFLECT_END()

// CameraController
ECS_REFLECT_BEGIN(ecs::CameraController)
ECS_REFLECT_FIELD(mode),
ECS_REFLECT_FIELD(moveSpeed),
ECS_REFLECT_FIELD(mouseSensitivity),
ECS_REFLECT_FIELD(yaw),
ECS_REFLECT_FIELD(pitch),
ECS_REFLECT_FIELD(orbitTarget),
ECS_REFLECT_FIELD(orbitDistance),
ECS_REFLECT_FIELD(orbitYaw),
ECS_REFLECT_FIELD(orbitPitch),
ECS_REFLECT_FIELD(invertY)
ECS_REFLECT_END()

// Mesh [ToDo] これはまた調整しなければ...
#ifdef ECS_MESH_REFLECTABLE
ECS_REFLECT_BEGIN(ecs::Mesh)
ECS_REFLECT_FIELD(meshType)
ECS_REFLECT_END()
#endif

// コンポーネントのリフレクション情報
ECS_REFLECT_BEGIN(ecs::Collider)
ECS_REFLECT_FIELD(type),
ECS_REFLECT_FIELD(isTrigger),
ECS_REFLECT_FIELD(isStatic)
ECS_REFLECT_END()