/**
 * @file TitleSceneSystem.cpp
 * @brief タイトルシーンのふるまい
 * @author Arima Keita
 * @date 2025-09-22
 */

 // ---------- インクルード ---------- // 
#include <Game/Systems/Scenes/TitleSceneSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Scene/SceneManager.h>

#include <Game/Components/Core/Transform.h>
#include <Game/Components/Render/Light.h>

#include <Debug/DebugUI.h>

namespace ecs {
	/**
	 * @brief コンストラクタ
	 * @param _desc
	 */
	TitleSceneSystem::TitleSceneSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
	}

	void TitleSceneSystem::Init()
	{
		Signature sig;
		sig.set(ecs_.GetComponentType<Transform>());
		sig.set(ecs_.GetComponentType<LightCommon>());
		sig.set(ecs_.GetComponentType<SpotLight>());
		ecs_.SetSystemSignature<TitleSceneSystem>(sig);
	}

	/**
	 * @brief 更新
	 *
	 * ここにタイトルシーンのふるまいを実装する
	 *
	 * @param _dt デルタタイム
	 */
	void TitleSceneSystem::Update(float _dt)
	{
		using namespace DirectX;

		if (scene_manager_.GetActiveScene() != "TitleScene") { return; }


		// 時間蓄積
		time_sec_ += _dt;

		if (entities_.empty()) { return; }

		// 全体デフォルト（必要なら調整）
		const float globalYawSpeed = 0.8f;   // 基本回転速度(rad/s)
		const float globalPitchFreq = 0.5f;  // 基本ピッチ周波数(Hz)
		const float globalPitchAmpDeg = 15.f;// 基本ピッチ振幅(deg)
		const float globalBasePitchDeg = -20.f; // 基本下向きオフセット(deg)
		const float yawOffsetRangeDeg = 60.f; // エンティティ毎のyaw振れ幅(deg)
		const float basePitchJitterDeg = 10.f; // basePitchの個別差(deg)

		for (auto& e : entities_) {
			// 必須コンポーネントを取得
			if (!ecs_.HasComponent<ecs::Transform>(e)) { continue; }
			if (!ecs_.HasComponent<ecs::LightCommon>(e)) { continue; }

			auto& tf = ecs_.GetComponent<ecs::Transform>(e);


			// TitleMotion が無い場合: for 文内でエンティティごとの差を生成する（決定論的）
			// Entity の識別子を使って簡易ハッシュを作る（同じエンティティで常に同じ差が出る）
			uint32_t seed = 0u;
			// ここでは Entity が public な id_ を持つ前提（既存コードで使用されているため）
			seed = static_cast<uint32_t>(e.id_);

			// Knuth の乗算ハッシュで拡散
			uint32_t h = seed * 2654435761u;
			// 0..1 に正規化
			const float t = static_cast<float>(h) / static_cast<float>(0xFFFFFFFFu);

			// 個別パラメータを決定
			const float yawOffsetDeg = (t - 0.5f) * yawOffsetRangeDeg;           // -range/2 .. +range/2
			const float basePitchDeg = globalBasePitchDeg + (t - 0.5f) * basePitchJitterDeg; // 微妙に上下
			const float yawSpeed = globalYawSpeed * (0.7f + 0.6f * t);          // 0.7..1.3倍ぐらい
			const float pitchFreq = globalPitchFreq * (0.6f + 0.8f * t);        // 0.6..1.4倍
			const float pitchAmpDeg = globalPitchAmpDeg * (0.6f + 0.8f * (1.0f - t)); // 少し逆相にしてバラつき
			const float phase = t * 6.28318548f; // 0..2pi の位相差

			const float pitchAmp = XMConvertToRadians(pitchAmpDeg);
			const float basePitch = XMConvertToRadians(basePitchDeg);
			const float yaw = time_sec_ * yawSpeed + XMConvertToRadians(yawOffsetDeg) + phase;
			const float pitch = basePitch + std::sinf(time_sec_ * pitchFreq + phase) * pitchAmp;

			const float cy = std::cosf(yaw);
			const float sy = std::sinf(yaw);
			const float cp = std::cosf(pitch);
			const float sp = std::sinf(pitch);

			// 球面座標から方向ベクトルを作る（Yが上下）
			DirectX::XMFLOAT3 dir{
				sy * cp, // x
				sp,      // y
				cy * cp  // z
			};

			// Transform をライト方向に向ける
			tf.SetRotationFromDirection(dir);
		}
	}


	void TitleSceneSystem::DebugTitleScene()
	{
	}
}
