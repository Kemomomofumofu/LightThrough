/**
 * @file PlayerControllerSystem.h
 * @brief プレイヤーを操作するシステム
 */

 // ---------- インクルード ---------- // 
#include <Game/Systems/PlayerControllerSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/InputSystem/InputSystem.h>

#include <Game/Components/Input/PlayerController.h>
#include <Game/Components/Core/Transform.h>
#include <Game/Components/Physics/Rigidbody.h>
#include <Game/Components/Physics/GroundContact.h>
#include <Game/Components/Input/MoveDirectionSource.h>
#include <Game/Components/Input/CameraController.h>

#include <DX3D/Math/MathUtils.h>
#include <Debug/Debug.h>
namespace ecs {
	//! @brief 初期化
	void PlayerControllerSystem::Init()
	{
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<PlayerController>());
		signature.set(ecs_.GetComponentType<Rigidbody>());
		signature.set(ecs_.GetComponentType<MoveDirectionSource>());
		signature.set(ecs_.GetComponentType<GroundContact>());
		ecs_.SetSystemSignature<PlayerControllerSystem>(signature);
	}

	//! @brief 更新
	void PlayerControllerSystem::Update(float _dt)
	{
		auto& input = input::InputSystem::Get();

		// 移動入力取得
		if (input.IsKeyDown('W')) { move_forward_ = true; }
		if (input.IsKeyDown('S')) { move_back_ = true; }
		if (input.IsKeyDown('A')) { move_left_ = true; }
		if (input.IsKeyDown('D')) { move_right_ = true; }

		// ジャンプ入力取得
		if (input.IsKeyTrigger(VK_SPACE)) { move_jump_ = true; }
	}

	//! @brief 固定更新
	void PlayerControllerSystem::FixedUpdate(float _fixedDt)
	{
		using namespace DirectX;

		// 移動処理
		for (auto e : entities_) {
			auto& tf = ecs_.GetComponent<Transform>(e);
			auto& pc = ecs_.GetComponent<PlayerController>(e);
			auto& rb = ecs_.GetComponent<Rigidbody>(e);
			auto& mds = ecs_.GetComponent<MoveDirectionSource>(e);
			auto& gc = ecs_.GetComponent<GroundContact>(e);

			float yaw = 0.0f;
			if (mds.target.IsInitialized() && ecs_.HasComponent<CameraController>(mds.target)) {
				yaw = ecs_.GetComponent<CameraController>(mds.target).yaw;
			}
			else {
				yaw = tf.GetYaw();
			}

			const XMFLOAT3& forward = { std::sinf(yaw), 0.0f, std::cosf(yaw) };
			const XMFLOAT3& right = { std::cosf(yaw), 0.0f, -std::sinf(yaw) };

			// 移動方向計算
			XMFLOAT3 moveDir{ 0.0f, 0.0f, 0.0f };
			if (move_forward_) { moveDir = math::Add(moveDir, forward); }
			if (move_back_) { moveDir = math::Sub(moveDir, forward); }
			if (move_right_) { moveDir = math::Add(moveDir, right); }
			if (move_left_) { moveDir = math::Sub(moveDir, right); }

			XMVECTOR dir = XMLoadFloat3(&moveDir);
			if (XMVectorGetX(XMVector3LengthSq(dir)) > 1e-4f) {
				dir = XMVector3Normalize(dir);
				float speed = pc.moveSpeed * _fixedDt;

				XMFLOAT3 delta{};
				XMStoreFloat3(&delta, dir * speed);
				rb.linearVelocity = math::Add(rb.linearVelocity, delta);
			}

			// ジャンプ処理
			if (move_jump_) {
				if (gc.isGrounded) {
					rb.linearVelocity.y = pc.jumpForce;
				}
			}

		}

		// フラグリセット
		move_forward_ = false;
		move_back_ = false;
		move_left_ = false;
		move_right_ = false;
		move_jump_ = false;

	}
} // namespace ecs