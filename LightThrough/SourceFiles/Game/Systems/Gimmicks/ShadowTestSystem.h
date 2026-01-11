#pragma once
/**
 * @file ShadowTestSystem.h
 * @brief 描画システム
 */

 // ---------- インクルード ---------- //
#include <unordered_map>
#include <wrl/client.h>

#include <Game/ECS/ISystem.h>

// ---------- 前方宣言 ---------- //
namespace dx3d {
	class GraphicsEngine;
}

namespace ecs {
	class LightDepthRenderSystem;
	class DebugRenderSystem;
	//! @brief 影判定結果
	struct ShadowTestResult {
		bool aInShadow = false; // Entity Aが影の中にいるか
		bool bInShadow = false; // Entity Bが影の中にいるか
	};


	class ShadowTestSystem : public ISystem {
	public:

		explicit ShadowTestSystem(const SystemDesc& _desc);
		//! @brief 初期化
		void Init() override;
		//! @brief 更新
		void FixedUpdate(float _dt) override;
		//! @brief Engineのセット
		void SetGraphicsEngine(dx3d::GraphicsEngine& _engine) { engine_ = &_engine; }
		
		/**
		 * @brief 影判定の結果を取得
		 * @param[in] _a: エンティティA
		 * @param[in] _b: エンティティB
		 * @param[out] _outResult: 影判定結果の出力先
		 * @return true: , false: 
		 */
		bool GetShadowTestResult(Entity _a, Entity _b, ShadowTestResult& _outResult) const;

		/**
		 * @brief 衝突ペアの登録
		 * @param _a: エンティティA
		 * @param _b: エンティティB
		 * @param _contactPoint: 衝突点
		 */
		void RegisterCollisionPair(Entity _a, Entity _b, const DirectX::XMFLOAT3& _contantPoint);

		//! @brief 保留中のテストをクリア
		void ClearPendingTests() { pending_tests_.clear(); }

		//! @brief 両方とも影の中にいるか
		bool AreBothInShadow(Entity _a, Entity _b) const;

	private:
		//! @brief コンピュート用リソースの作成
		void CreateComputeResources();
		//! @brief 影判定の実行
		void ExecuteShadowTests();
		//! @brief テスト用のポイント収集
		void CollectTestPoints(Entity _entity, std::vector<DirectX::XMFLOAT3>& _outPoints);

		//! @brief CS用定数バッファ
		struct alignas(16) CSParams {
			DirectX::XMMATRIX lightViewProj;
			uint32_t numPoints;
			uint32_t shadowWidth;
			uint32_t shadowHeight;
			uint32_t sliceIndex;
			float depthBias;
			uint32_t _pad0;
		};

		//! @brief 衝突ペアキー
		struct PairKey {
			Entity a;
			Entity b;
			bool operator==(const PairKey& _o) const {
				return(a == _o.a && b == _o.b) || (a == _o.b && b == _o.a);
			}
		};
		struct PairKeyHash {
			size_t operator()(const PairKey& _k) const {
				auto h1 = std::hash<Entity>{}(_k.a);
				auto h2 = std::hash<Entity>{}(_k.b);
				return h1 ^ (h2 << 1);
			}
		};

		//! @brief 衝突情報
		struct PendingTest {
			Entity a;
			Entity b;
			DirectX::XMFLOAT3 contactPoint;
			size_t pointStartIndex = 0;
			size_t pointCountA = 0;
			size_t pointCountB = 0;
		};

	private:
		dx3d::GraphicsEngine* engine_{};
		std::weak_ptr<LightDepthRenderSystem> light_depth_system_{};
		std::weak_ptr<DebugRenderSystem> debug_render_system_{};

		// コンピュートシェーダー関連
		dx3d::ConstantBufferPtr cb_params_{};
		dx3d::StructuredBufferPtr point_buffer_{};
		dx3d::RWStructuredBufferPtr result_buffer_{};
		dx3d::StagingBufferPtr staging_buffer_{};

		// 処理対象
		std::vector<PendingTest> pending_tests_{};
		std::unordered_map<PairKey, ShadowTestResult, PairKeyHash> shadow_results_{};

		static constexpr uint32_t MAX_TEST_POINTS = 4096;
		static constexpr float DEPTH_BIAS = 0.005f;
		static constexpr uint32_t POINTS_PER_AABB = 8;
		static constexpr uint32_t CS_THREAD_GROUP_SIZE = 64;

	};

}