/**
 * @file ShadowTestSystem.h
 * @brief 影なのか判定するシステム
 */

 // ---------- インクルード ---------- // 
#include <vector>
#include <Game/Systems/Gimmicks/ShadowTestSystem.h>

#include <DX3D/Graphics/GraphicsEngine.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Graphics/Buffers/ConstantBuffer.h>

#include <Game/ECS/Coordinator.h>
#include <Game/Systems/Renderers/LightDepthRenderSystem.h>
#include <Game/Components/Core/Transform.h>
#include <Game/Components/Render/Light.h>
#include <Game/Components/Physics/Collider.h>

#include <Game/Collisions/CollisionUtils.h>

#include <Debug/Debug.h>
#include <Debug/DebugUI.h>
#include <Game/Systems/Renderers/DebugRenderSystem.h>



namespace ecs {

	//! @brief コンストラクタ
	ShadowTestSystem::ShadowTestSystem(const SystemDesc& _desc)
		:ISystem(_desc)
	{
	}

	//! @brief 初期化
	void ShadowTestSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		signature.set(ecs_.GetComponentType<LightCommon>());
		ecs_.SetSystemSignature<ShadowTestSystem>(signature);

		// コンピュート用リソース作成
		CreateComputeResources();

		light_depth_system_ = ecs_.GetSystem<LightDepthRenderSystem>();
		debug_render_system_ = ecs_.GetSystem<DebugRenderSystem>();
	}

	//! @brief 更新
	void ShadowTestSystem::FixedUpdate(float _dt)
	{
		ExecuteShadowTests();
	}

	//! @brief 影判定結果の取得
	bool ShadowTestSystem::GetShadowTestResult(Entity _a, Entity _b, ShadowTestResult& _outResult) const
	{
		PairKey key{ _a, _b };
		auto it = shadow_results_.find(key);
		if (it != shadow_results_.end())
		{
			_outResult = it->second;
			return true;
		}
		return false;
	}

	//! @brief 衝突ペアの登録
	void ShadowTestSystem::RegisterCollisionPair(Entity _a, Entity _b, const DirectX::XMFLOAT3& _contactPoint)
	{
		if (pending_tests_.size() >= MAX_TEST_POINTS) return;

		pending_tests_.push_back(PendingTest{ _a, _b, _contactPoint });
	}

	//! @brief 両方とも影の中にいるか
	bool ShadowTestSystem::AreBothInShadow(Entity _a, Entity _b) const
	{
		ShadowTestResult result{};
		if (GetShadowTestResult(_a, _b, result)) {
			return result.aInShadow && result.bInShadow;
		}
		return false;
	}

	//! @brief コンピュート用リソース作成
	void ShadowTestSystem::CreateComputeResources()
	{
		if (!engine_) return;


		auto& device = engine_->GetGraphicsDevice();
		// 定数バッファ
		cb_params_ = device.CreateConstantBuffer({ sizeof(CSParams), nullptr });
		// 入力ポイント用
		point_buffer_ = device.CreateStructuredBuffer({ sizeof(XMFLOAT3), MAX_TEST_POINTS, nullptr });
		// 出力フラグ用
		result_buffer_ = device.CreateRWStructuredBuffer({ sizeof(uint32_t), MAX_TEST_POINTS });
		// 読み戻し用
		staging_buffer_ = device.CreateStagingBuffer({ sizeof(uint32_t), MAX_TEST_POINTS });
	}

	//! @brief 影判定の実行
	void ShadowTestSystem::ExecuteShadowTests()
	{
		auto lightDepthSystem = light_depth_system_.lock();
		if (!engine_ || !lightDepthSystem) { return; }

		if (pending_tests_.empty()) { return; }

		// ライトがない場合はすべて影の中とする
		if (entities_.empty()) {
			for (const auto& test : pending_tests_) {
				PairKey key{ test.a, test.b };
				shadow_results_[key] = ShadowTestResult{ true, true };
			}
			pending_tests_.clear();
			return;
		}

		auto& device = engine_->GetGraphicsDevice();
		auto& deferredContext = engine_->GetDeferredContext();

		// ComputeShader取得
		auto& shaderCache = engine_->GetShaderCache();
		auto& csEntry = shaderCache.GetCS(dx3d::ComputeShaderKind::ShadowTest);

		if (!csEntry.shader)
		{
			DebugLogWarning("[ShadowTestSystem] shaderが未設定");
			return;
		}

		shadow_results_.clear();

		// ポイントデータを収集
		std::vector<DirectX::XMFLOAT3> testPoints;
		testPoints.reserve(pending_tests_.size() * POINTS_PER_AABB * 2);

		for (auto& test : pending_tests_) {
			test.pointStartIndex = testPoints.size();

			size_t beforePointsSize = testPoints.size();
			CollectTestPoints(test.a, testPoints);
			test.pointCountA = testPoints.size() - beforePointsSize;

			beforePointsSize = testPoints.size();
			CollectTestPoints(test.b, testPoints);
			test.pointCountB = testPoints.size() - beforePointsSize;
		}

		if (testPoints.empty()) { return; }

		// ポイントバッファの更新
		point_buffer_->Update(deferredContext, testPoints.data(), sizeof(DirectX::XMFLOAT3) * testPoints.size());

		// 判定
		std::vector<uint32_t> accumulatedResults(testPoints.size(), 0);

		for (const auto& lightEntity : entities_) {
			auto& common = ecs_.GetComponent<LightCommon>(lightEntity);
			if (!common.enabled) { continue; }

			auto& lightTf = ecs_.GetComponent<Transform>(lightEntity);

			DirectX::XMMATRIX lightViewProj{};
			int shadowIndex = 0;
			lightDepthSystem->GetShadowInfo(lightEntity, shadowIndex, lightViewProj);

			// 定数バッファの更新
			CSParams params{};
			params.lightViewProj = lightViewProj;
			params.numPoints = static_cast<uint32_t>(testPoints.size());
			params.shadowWidth = lightDepthSystem->GetShadowMapWidth();
			params.shadowHeight = lightDepthSystem->GetShadowMapHeight();
			params.sliceIndex = static_cast<uint32_t>(shadowIndex);
			params.depthBias = DEPTH_BIAS;

			cb_params_->Update(deferredContext, &params, sizeof(CSParams));


			// リソースセット
			deferredContext.CSSetShader(csEntry.shader.Get());
			deferredContext.CSSetConstantBuffer(0, *cb_params_);
			ID3D11ShaderResourceView* srvs[] = {
				point_buffer_->GetSRV(),
				lightDepthSystem->GetShadowMapSRV()
			};
			deferredContext.CSSetShaderResources(0, 2, srvs);
			deferredContext.CSSetUnorderedAccessView(0, result_buffer_.get());
			// 実行
			uint32_t groupCount = static_cast<uint32_t>(testPoints.size() + CS_THREAD_GROUP_SIZE - 1) / CS_THREAD_GROUP_SIZE; // memo: 
			deferredContext.Dispatch(groupCount, 1, 1);
			// 結果の取得
			deferredContext.CopyResource(*staging_buffer_, *result_buffer_);
			void* mappedData = staging_buffer_->Map();
			if (mappedData) {
				auto* flags = static_cast<const uint32_t*>(mappedData);
				for (size_t i = 0; i < testPoints.size(); ++i) {
					if (flags[i] == 0) {
						accumulatedResults[i] = 1;
					}
				}
				staging_buffer_->Unmap();
			}
		}
		// クリア
		deferredContext.CSClearResources(2, 1);

		// デバッグ描画用のシステム取得
		auto debugRenderSystem = debug_render_system_.lock();

		// 結果の格納 & デバッグ描画
		for (const auto& test : pending_tests_) {
			PairKey key{ test.a, test.b };

			size_t aInShadowCount = 0;
			for (size_t i = 0; i < test.pointCountA; ++i) {
				if (accumulatedResults[test.pointStartIndex + i] == 0) {
					++aInShadowCount;
				}
			}

			size_t bInShadowCount = 0;
			for (size_t i = 0; i < test.pointCountB; ++i) {
				if (accumulatedResults[test.pointStartIndex + test.pointCountA + i] == 0) {
					++bInShadowCount;
				}
			}

			ShadowTestResult result{};
			result.aInShadow = (test.pointCountA > 0) && (aInShadowCount > test.pointCountA / 2);
			result.bInShadow = (test.pointCountB > 0) && (bInShadowCount > test.pointCountB / 2);

			shadow_results_[key] = result;
		}

		pending_tests_.clear();
	}

	//! @brief テストポイントの収集
	void ShadowTestSystem::CollectTestPoints(Entity _entity, std::vector<DirectX::XMFLOAT3>& _outPoints)
	{
		if (ecs_.HasComponent<Collider>(_entity)) {
			auto& col = ecs_.GetComponent<Collider>(_entity);

			switch (col.type) {
			case collision::ShapeType::Box: {
				DirectX::XMFLOAT3 corners[8];
				collision::GetOBBCorners(col.worldOBB, corners);
				for (int i = 0; i < 8; ++i) {
					_outPoints.push_back(corners[i]);
				}
				break;
			}
			case collision::ShapeType::Sphere: {
				collision::GetSphereSamplePoints(col.worldSphere, _outPoints, true);
				break;
			}
			default:
				break;
			}
		}
	}
}
