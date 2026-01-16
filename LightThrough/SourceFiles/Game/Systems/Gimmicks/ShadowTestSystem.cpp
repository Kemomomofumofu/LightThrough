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

		// デバッグUI登録
#if defined(DEBUG) || defined(_DEBUG)
		debug::DebugUI::ResistDebugFunction([this]()
			{
				if (ImGui::Begin("Shadow Test Debug")) {
					ImGui::Checkbox("Show Test Points", &show_debug_points_);
					ImGui::Text("Test Points Count: %zu", debug_test_points_.size());

					// 影の中のポイント数をカウント
					size_t shadowCount = 0;
					for (const auto& point : debug_test_points_) {
						if (point.isInShadow) shadowCount++;
					}
					ImGui::Text("Points in Shadow: %zu", shadowCount);
					ImGui::Text("Points in Light: %zu", debug_test_points_.size() - shadowCount);
				}
				ImGui::End();
			}
		);
#endif
	}

	//! @brief 更新
	void ShadowTestSystem::FixedUpdate(float _dt)
	{
		ExecuteShadowTests();
	}

	//! @brief 更新
	void ShadowTestSystem::Update(float _dt)
	{
#if defined(DEBUG) || defined(_DEBUG)
		// デバッグ表示
		if (show_debug_points_) {
			auto debugRenderSystem = debug_render_system_.lock();
			if (debugRenderSystem) {
				constexpr float POINT_SIZE = 0.05f;
				constexpr DirectX::XMFLOAT4 LIGHT_COLOR = { 0.0f, 1.0f, 0.0f, 1.0f };  // 緑（光の中）
				constexpr DirectX::XMFLOAT4 SHADOW_COLOR = { 1.0f, 0.0f, 0.0f, 1.0f }; // 赤（影の中）

				for (const auto& point : debug_test_points_) {
					debugRenderSystem->DrawPoint(
						point.position,
						point.isInShadow ? SHADOW_COLOR : LIGHT_COLOR,
						POINT_SIZE
					);
				}
			}
		}
#endif
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

#if defined(DEBUG) || defined(_DEBUG)
		// 衝突がない場合はデバッグポイントをクリア
		if (pending_tests_.empty()) {
			debug_test_points_.clear();
			return;
		}
#else
		if (pending_tests_.empty()) { return; }
#endif

		// ライトがない場合はすべて影の中とする
		if (entities_.empty()) {
			for (const auto& test : pending_tests_) {
				PairKey key{ test.a, test.b };
				shadow_results_[key] = ShadowTestResult{ true, true };
			}
			pending_tests_.clear();
#if defined(DEBUG) || defined(_DEBUG)
			debug_test_points_.clear();
#endif
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

		if (testPoints.empty()) {
#if defined(DEBUG) || defined(_DEBUG)
			debug_test_points_.clear();
#endif
			return;
		}

		// ポイントバッファの更新
		point_buffer_->Update(deferredContext, testPoints.data(), sizeof(DirectX::XMFLOAT3) * testPoints.size());

		// 判定
		// どれか1つのライトでも照らされていれば false (照らされている)、すべてのライトで照らされていなければ true (影の中)
		std::vector<bool> isLitByAnyLight(testPoints.size(), false);

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
			// ライト情報
			params.lightPos = lightTf.position;
			params.lightDir = lightTf.GetForward();
			params.cosOuterAngle = -1.0f;
			params.cosInnerAngle = -1.0f;
			params.lightRange = 100000.0f;
			// スポットライトなら
			if (ecs_.HasComponent<SpotLight>(lightEntity)) {
				auto& spot = ecs_.GetComponent<SpotLight>(lightEntity);
				params.cosOuterAngle = spot.outerCos;
				params.cosInnerAngle = spot.innerCos;
				params.lightRange = spot.range;
			}

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

			// シャドウサンプラーをセット
			ID3D11SamplerState* samplers[] = { lightDepthSystem->GetShadowSampler() };
			deferredContext.GetDeferredContext()->CSSetSamplers(0, 1, samplers);

			// 実行
			uint32_t groupCount = static_cast<uint32_t>(testPoints.size() + CS_THREAD_GROUP_SIZE - 1) / CS_THREAD_GROUP_SIZE;
			deferredContext.Dispatch(groupCount, 1, 1);

			// 結果の取得
			deferredContext.CopyResource(*staging_buffer_, *result_buffer_);
			void* mappedData = staging_buffer_->Map();
			if (mappedData) {

				// デバッグ用カウンタ
				uint32_t countLit = 0;
				uint32_t countShadow = 0;
				uint32_t countOutsideXY = 0;
				uint32_t countBadW = 0;

				auto* flags = static_cast<const uint32_t*>(mappedData);
				for (size_t i = 0; i < testPoints.size(); ++i)
				{
					switch (flags[i])
					{
					case 0: // lit
						++countLit;
						isLitByAnyLight[i] = true;
						break;
					case 1: // shadow
						++countShadow;
						break;
					case 2: // outside XY
						++countOutsideXY;
						break;
					case 3: // w == 0
						++countBadW;
						break;
					default:
						break;
					}
				}
				staging_buffer_->Unmap();

#if defined(DEBUG) || defined(_DEBUG)
				GameLogFInfo("[ShadowTestSystem] Light Entity {}: lit={}, shadow={}, outsideXY={}, badW={}",
					lightEntity.id_,
					countLit,
					countShadow,
					countOutsideXY,
					countBadW
				);
#endif // DEBUG
			}
		}
		// クリア
		deferredContext.CSClearResources(2, 1);

		// 結果の格納
		for (const auto& test : pending_tests_) {
			PairKey key{ test.a, test.b };

			// エンティティAの判定: すべてのポイントが照らされていない場合に影とみなす
			size_t aLitCount = 0;
			for (size_t i = 0; i < test.pointCountA; ++i) {
				if (isLitByAnyLight[test.pointStartIndex + i]) {
					++aLitCount;
				}
			}

			// エンティティBの判定: すべてのポイントが照らされていない場合に影とみなす
			size_t bLitCount = 0;
			for (size_t i = 0; i < test.pointCountB; ++i) {
				if (isLitByAnyLight[test.pointStartIndex + test.pointCountA + i]) {
					++bLitCount;
				}
			}

			ShadowTestResult result{};
			// すべてのポイントが照らされていない = 影の中
			result.aInShadow = (test.pointCountA > 0) && (aLitCount == 0);
			result.bInShadow = (test.pointCountB > 0) && (bLitCount == 0);

			shadow_results_[key] = result;
		}

		pending_tests_.clear();

#if defined(DEBUG) || defined(_DEBUG)
		// デバッグ表示用の情報を更新
		UpdateDebugVisualization(testPoints, isLitByAnyLight);
#endif
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

	//! @brief デバッグ表示用情報の更新
	void ShadowTestSystem::UpdateDebugVisualization(const std::vector<DirectX::XMFLOAT3>& _testPoints,
		const std::vector<bool>& _isLitByAnyLight)
	{
		debug_test_points_.clear();
		debug_test_points_.reserve(_testPoints.size());

		for (size_t i = 0; i < _testPoints.size(); ++i) {
			DebugTestPoint debugPoint;
			debugPoint.position = _testPoints[i];
			// isLitByAnyLight が false の場合は影の中
			debugPoint.isInShadow = !_isLitByAnyLight[i];
			debug_test_points_.push_back(debugPoint);
		}
	}
}
