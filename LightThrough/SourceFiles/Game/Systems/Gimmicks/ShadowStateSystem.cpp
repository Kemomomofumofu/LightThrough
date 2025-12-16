/**
 * @file ShadowStateSystem.cpp
 */

 // ---------- インクルード ---------- //
#include <Game/Systems/Gimmicks/ShadowStateSystem.h>

#include <DirectXMath.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Collider.h>
#include <Game/Components/Gimmicks/ShadowState.h>
#include <Game/Components/Light.h>


namespace ecs {
	/**
	 * @brief コンストラクタ
	 */
	ShadowStateSystem::ShadowStateSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
	}

	/**
	 * @brief 初期化
	 */
	void ShadowStateSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<Collider>());
		signature.set(ecs_.GetComponentType<ShadowState>());
		ecs_.SetSystemSignature<ShadowStateSystem>(signature);
	}

	/**
	 * @brief 更新処理
	 */
	void ShadowStateSystem::Update(float _dt)
	{
		using namespace DirectX;
		if (entities_.empty()) { return; }

		auto lights = ecs_.GetEntitiesWithComponent<LightCommon>();
		if (lights.empty()) { return; }

		for (auto& e : entities_) {
			auto& shadowState = ecs_.GetComponent<ShadowState>(e);
			auto& transform = ecs_.GetComponent<Transform>(e);
			bool isInShadow = false;


			for (auto& lightE : lights) {
				auto& lightCommon = ecs_.GetComponent<LightCommon>(lightE);
				auto& lightTf = ecs_.GetComponent<Transform>(lightE);
				SpotLight* spotLight = nullptr;
				if(ecs_.HasComponent<SpotLight>(lightE)) {
					spotLight = &ecs_.GetComponent<SpotLight>(lightE);
				}
				if (!lightCommon.enabled) { continue; }

				XMMATRIX lightVP = BuildLightViewProj(lightTf, spotLight);
				XMVECTOR pos = XMLoadFloat3(&transform.position);

				break;
			}
			shadowState.IsInShadow = isInShadow;
		}
	}

} // namespace ecs