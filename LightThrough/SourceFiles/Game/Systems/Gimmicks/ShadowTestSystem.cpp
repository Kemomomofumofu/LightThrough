/**
 * @file ShadowTestSystem.cpp
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
	void ShadowTestSystem::Update(float _dt)
	{
#if defined(DEBUG) || defined(_DEBUG)

		DebugCheckSliceIndex();
		
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

	void ShadowTestSystem::FixedUpdate(float _fixedDt)
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
		if (pending_contact_points_.size() >= MAX_TEST_POINTS) return;

		PairKey key{ _a, _b };

		// 既存のペアを検索
		auto it = std::find_if(pending_tests_.begin(), pending_tests_.end(),
			[&key](const PendingTest& test) {
				return PairKey{ test.a, test.b } == key;
			});

		if (it != pending_tests_.end()) {
			// 既存のペアに接触点を追加
			it->contactPointCount++;
		}
		else {
			// 新しいペアを追加
			PendingTest newTest{};
			newTest.a = _a;
			newTest.b = _b;
			newTest.contactPointStartIndex = pending_contact_points_.size();
			newTest.contactPointCount = 1;
			pending_tests_.push_back(newTest);
		}

		// 接触点を追加
		pending_contact_points_.push_back(_contactPoint);
	}

	//! @brief 両方とも影の中にいるか（接触点がすべて影の中か）
	bool ShadowTestSystem::AreBothInShadow(Entity _a, Entity _b) const
	{
		ShadowTestResult result{};
		if (GetShadowTestResult(_a, _b, result)) {
			// 接触点がすべて影の中の場合にtrueを返す
			return result.allContactPointsInShadow;
		}
		return false;
	}

	//! @brief コンピュート用リソース作成
	void ShadowTestSystem::CreateComputeResources()
	{
		auto& device = engine_.GetGraphicsDevice();
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

		// 衝突がない場合は前フレームの結果を維持
		if (pending_tests_.empty()) {
			pending_contact_points_.clear();
#if defined(DEBUG) || defined(_DEBUG)
			debug_test_points_.clear();
#endif
			return;
		}

		// 新しいテストがある場合のみ結果をクリア
		shadow_results_.clear();

		// ライトがない場合はすべて影の中とする
		if (entities_.empty()) {
			for (const auto& test : pending_tests_) {
				PairKey key{ test.a, test.b };
				ShadowTestResult result{};
				result.aInShadow = true;
				result.bInShadow = true;
				result.allContactPointsInShadow = true;
				shadow_results_[key] = result;
			}
#if defined(DEBUG) || defined(_DEBUG)
			// ライトがない場合はすべて影として表示
			debug_test_points_.clear();
			for (const auto& point : pending_contact_points_) {
				debug_test_points_.push_back({ point, true });
			}
#endif
			pending_tests_.clear();
			pending_contact_points_.clear();
			return;
		}

		auto& device = engine_.GetGraphicsDevice();
		auto* immediateContext = engine_.GetImmediateContext();

		// ComputeShader取得
		auto& shaderCache = engine_.GetShaderCache();
		auto& csEntry = shaderCache.GetCS(dx3d::ComputeShaderKind::ShadowTest);

		// 接触点をテストポイントとして使用
		std::vector<DirectX::XMFLOAT3> testPoints = pending_contact_points_;

		if (testPoints.empty()) {
#if defined(DEBUG) || defined(_DEBUG)
			debug_test_points_.clear();
#endif
			pending_tests_.clear();
			pending_contact_points_.clear();
			return;
		}

		// ポイントバッファの更新
		point_buffer_->Update(testPoints.data(), sizeof(DirectX::XMFLOAT3) * testPoints.size());
		// 判定
		// memo: falseで初期化。CSの結果が光ならtrueで上書きする。
		std::vector<bool> isLitByAnyLight(testPoints.size(), false);

		const auto& shadowLights = lightDepthSystem->GetShadowLights();
		for (uint32_t i = 0; i < shadowLights.size(); ++i) {
			const auto& entry = shadowLights[i];

			// 定数バッファの更新
			CSParams params{};
			params.lightViewProj = entry.lightViewProj;
			params.numPoints = static_cast<uint32_t>(testPoints.size());
			params.shadowWidth = lightDepthSystem->GetShadowMapWidth();
			params.shadowHeight = lightDepthSystem->GetShadowMapHeight();
			params.sliceIndex = entry.sliceIndex;
			// ライト情報
			auto lightTf = ecs_.GetComponent<Transform>(entry.light);
			auto lightCommon = ecs_.GetComponent<LightCommon>(entry.light);
			params.lightPos = lightTf->GetWorldPosition();
			params.lightDir = lightTf->GetWorldForwardCached();
			params.cosOuterAngle = -1.0f;
			params.cosInnerAngle = -1.0f;
			params.lightRange = 100000.0f;
			// スポットライトなら
			if (ecs_.HasComponent<SpotLight>(entry.light)) {
				auto spot = ecs_.GetComponent<SpotLight>(entry.light);
				params.cosOuterAngle = spot->outerCos;
				params.cosInnerAngle = spot->innerCos;
				params.lightRange = spot->range;
			}
			// CB更新
			D3D11_MAPPED_SUBRESOURCE mapped{};
			immediateContext->Map(
				cb_params_->GetBuffer(),
				0,
				D3D11_MAP_WRITE_DISCARD,
				0,
				&mapped
			);
			memcpy(mapped.pData, &params, sizeof(CSParams));
			immediateContext->Unmap(cb_params_->GetBuffer(), 0);

			// リソースセット
			immediateContext->CSSetShader(csEntry.shader.Get(), nullptr, 0);
			// CB
			ID3D11Buffer* cb = cb_params_->GetBuffer();
			immediateContext->CSSetConstantBuffers(0, 1, &cb);
			// SRV
			ID3D11ShaderResourceView* csSrvs[2];
			csSrvs[0] = point_buffer_->GetSRV();
			csSrvs[1] = lightDepthSystem->GetShadowMapSRVs();
			immediateContext->CSSetShaderResources(0, 2, csSrvs);
			// UAV
			ID3D11UnorderedAccessView* uav = result_buffer_->GetUAV();
			UINT initialCounts = 0;
			immediateContext->CSSetUnorderedAccessViews(0, 1, &uav, &initialCounts);

			// シャドウサンプラーをセット
			ID3D11SamplerState* samplers[] = { lightDepthSystem->GetShadowSampler() };
			immediateContext->CSSetSamplers(0, 1, samplers);

			// 実行
			uint32_t groupCount = static_cast<uint32_t>(testPoints.size() + CS_THREAD_GROUP_SIZE - 1) / CS_THREAD_GROUP_SIZE;
			immediateContext->Dispatch(groupCount, 1, 1);

			// GPUを待ってから結果を取得
			immediateContext->CopyResource(staging_buffer_->GetBuffer(), result_buffer_->GetBuffer());
			void* mappedData = staging_buffer_->Map();
			if (mappedData) {

#if defined(DEBUG) || defined(_DEBUG)
				// デバッグ用カウンタ
				uint32_t countLit = 0;
				uint32_t countShadow = 0;
				uint32_t countOutUV = 0;
				uint32_t countOutZ = 0;
				uint32_t countZeroW = 0;
				uint32_t countOutRange = 0;
				uint32_t countOutSideCone = 0;
#endif // DEBUG
				// outFlags: 0 = lit, 1 = shadow, 2 = outUV, 3 = outZ, 4 = wZero, 5 = outRange, 6 = outSideCone
				auto* flags = static_cast<const uint32_t*>(mappedData);
				for (size_t j = 0; j < testPoints.size(); ++j)
				{
					switch (flags[j])
					{
					case 0: // lit
#if defied(DEBUG) || defined(_DEBUG)
						++countLit;
#endif // DEBUG
						isLitByAnyLight[j] = true;
						break;
#if defined(DEBUG) || defined(_DEBUG)
					case 1: // shadow
						++countShadow;
						break;
					case 2: // outUV
						++countOutUV;
						break;
					case 3: // outZ
						++countOutZ;
						break;
					case 4: // wZero
						++countZeroW;
						break;
					case 5: // outRange
						++countOutRange;
						break;
					case 6: // outSideCone
						++countOutSideCone;
						break;
#endif // DEBUG
					default:
						break;
					}
				}
				staging_buffer_->Unmap();

#if defined(DEBUG) || defined(_DEBUG)
				
				//DebugLogInfo("[ShadowTestSystem] Light Entity {}: lit={}, shadow={}, outUV={}, outZ={}, zeroW={}, outRange={}, outSideCone={}",
				//	entry.light.id_,
				//	countLit,
				//	countShadow,
				//	countOutUV,
				//	countOutZ,
				//	countZeroW,
				//	countOutRange,
				//	countOutSideCone
				//);
				
#endif // DEBUG
			}
		}
		// クリア
		ID3D11ShaderResourceView* nullSrvs[2] = { nullptr, nullptr };
		immediateContext->CSSetShaderResources(0, 2, nullSrvs);
		ID3D11UnorderedAccessView* nullUav = nullptr;
		immediateContext->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);

#if defined(DEBUG) || defined(_DEBUG)
		// デバッグ表示用の情報を更新
		UpdateDebugVisualization(testPoints, isLitByAnyLight);
#endif

		// 結果の格納
		for (const auto& test : pending_tests_) {
			PairKey key{ test.a, test.b };

			bool allContactPointsInShadow = true;
			size_t litCount = 0;

			for (size_t i = 0; i < test.contactPointCount; ++i) {
				size_t pointIndex = test.contactPointStartIndex + i;
				if (pointIndex < isLitByAnyLight.size() && isLitByAnyLight[pointIndex]) {
					allContactPointsInShadow = false;
					++litCount;
				}
			}
#if defined(DEBUG) || defined(_DEBUG)
			/*
			DebugLogInfo("[ShadowTestSystem] Entity pair ({}, {}): {}/{} points lit, allInShadow={}",
				test.a.id_, test.b.id_,
				litCount, test.contactPointCount,
				allContactPointsInShadow);
			*/
#endif // DEBUG || _DEBUG

			ShadowTestResult result{};
			result.allContactPointsInShadow = allContactPointsInShadow;
			result.aInShadow = allContactPointsInShadow;
			result.bInShadow = allContactPointsInShadow;

			shadow_results_[key] = result;
		}

		// 最後にクリア
		pending_tests_.clear();
		pending_contact_points_.clear();
	}

	//! @brief テストポイントの収集
	void ShadowTestSystem::CollectTestPoints(Entity _entity, std::vector<DirectX::XMFLOAT3>& _outPoints)
	{
		if (ecs_.HasComponent<Collider>(_entity)) {
			auto col = ecs_.GetComponent<Collider>(_entity);

			switch (col->type) {
			case collision::ShapeType::Box: {
				DirectX::XMFLOAT3 corners[8];
				collision::GetOBBCorners(col->worldOBB, corners);
				for (int i = 0; i < 8; ++i) {
					_outPoints.push_back(corners[i]);
				}
				break;
			}
			case collision::ShapeType::Sphere: {
				collision::GetSphereSamplePoints(col->worldSphere, _outPoints, true);
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

	void ShadowTestSystem::DebugCheckSliceIndex()
	{
#if defined(DEBUG) || defined(_DEBUG)
		auto lightDepthSystem = light_depth_system_.lock();
		if (!lightDepthSystem) return;

		const auto& lights = lightDepthSystem->GetShadowLights();
		for (uint32_t i = 0; i < lights.size(); ++i) {
			const auto& entry = lights[i];
			if (entry.sliceIndex != i) {
				DebugLogError("[SliceIndexMismatch] Light {} expectedSlice={} actualSlice={}",
					entry.light.id_, i, entry.sliceIndex);
			}
		}
#endif
	}
}
