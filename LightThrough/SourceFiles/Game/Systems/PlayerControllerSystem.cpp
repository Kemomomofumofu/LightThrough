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
#include <Game/Components/GamePlay/LightPlaceRequest.h>
#include <Game/Components/Render/Light.h>

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
		if (input.IsKeyTrigger(VK_SPACE)) { request_jump_ = true; }

		// ライト生成リクエスト取得
		if (input.IsMouseTrigger(input::MouseButton::Left)) { request_spawn_light_ = true; }
		if (input.IsMouseTrigger(input::MouseButton::Right)) { request_despawn_light_ = true; }

		//// 手持ちライト切り替え
		//if (input.IsMouseTrigger(input::MouseButton::Middle)) { toggle_hand_light_ = true; }
	}

	//! @brief 固定更新
	void PlayerControllerSystem::FixedUpdate(float _fixedDt)
	{
		using namespace DirectX;


		for (auto e : entities_) {
			auto tf = ecs_.GetComponent<Transform>(e);
			auto pc = ecs_.GetComponent<PlayerController>(e);
			auto rb = ecs_.GetComponent<Rigidbody>(e);
			auto mds = ecs_.GetComponent<MoveDirectionSource>(e);
			auto gc = ecs_.GetComponent<GroundContact>(e);

			float yaw = 0.0f;
			if (mds->target.IsInitialized() && ecs_.HasComponent<CameraController>(mds->target)) {
				yaw = ecs_.GetComponent<CameraController>(mds->target)->yaw;
			}
			else {
				yaw = tf->GetYaw();
			}

			const XMFLOAT3& forward = { std::sinf(yaw), 0.0f, std::cosf(yaw) };
			const XMFLOAT3& right = { std::cosf(yaw), 0.0f, -std::sinf(yaw) };


			// ---------- 移動 ---------- //
			XMFLOAT3 moveDir{ 0.0f, 0.0f, 0.0f };
			if (move_forward_) { moveDir = math::Add(moveDir, forward); }
			if (move_back_) { moveDir = math::Sub(moveDir, forward); }
			if (move_right_) { moveDir = math::Add(moveDir, right); }
			if (move_left_) { moveDir = math::Sub(moveDir, right); }

			XMVECTOR dir = XMLoadFloat3(&moveDir);
			if (XMVectorGetX(XMVector3LengthSq(dir)) > 1e-4f) {
				dir = XMVector3Normalize(dir);
				float speed = pc->moveSpeed * _fixedDt;

				XMFLOAT3 delta{};
				XMStoreFloat3(&delta, dir * speed);
				rb->linearVelocity = math::Add(rb->linearVelocity, delta);
			}

			// ---------- ジャンプ ---------- // 
			if (request_jump_) {
				if (gc->isGrounded) {
					rb->linearVelocity.y = pc->jumpForce;
				}
			}

			// ---------- ライト生成 ---------- // 
			if (request_spawn_light_) {
				Entity camEntity{};
				if (mds->target.IsInitialized()) {
					camEntity = mds->target;
				}
				else {
					camEntity = e;
				}

				LightPlaceRequest req{};
				auto camTf = ecs_.GetComponent<Transform>(camEntity);
				req.spawnPos = camTf->GetWorldPosition();
				req.spawnDir = camTf->GetWorldForwardCached();

				ecs_.RequestAddComponent<LightPlaceRequest>(e, req);
			}
			// ---------- ライト破棄 ---------- //
			if (request_despawn_light_) {
				if (pc->ownedLights.size() != 0) {
					Entity lightEntity = pc->ownedLights.front();
					pc->ownedLights.pop_front();
					ecs_.RequestDestroyEntity(lightEntity);
				}
			}


			// ---------- 手持ちライトON/OFF ---------- //
			if(toggle_hand_light_) {
				if (pc->handLight.IsInitialized() && ecs_.IsValidEntity(pc->handLight)) {
					auto lightCommon = ecs_.GetComponent<LightCommon>(pc->handLight);
					lightCommon->enabled = !lightCommon->enabled;
				}
				else {
					DebugLogError("[PlayerControllerSystem] 手持ちライトが存在しない。\n");
				}
			}
		}

		// フラグリセット
		move_forward_ = false;
		move_back_ = false;
		move_left_ = false;
		move_right_ = false;
		request_jump_ = false;
		request_spawn_light_ = false;
		request_despawn_light_ = false;
		toggle_hand_light_ = false;

	}
} // namespace ecs