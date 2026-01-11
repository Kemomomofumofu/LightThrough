
/**
 * @file TransformSystem.cpp
 * @brief TransformSystem
 * @author Arima Keita
 * @date 2025-10-03
 */

 // ---------- インクルード ---------- // 
#include <unordered_map>
#include <unordered_set>

#include <Game/Systems/TransformSystem.h>
#include <Game/ECS/Coordinator.h>
#include <Game/Components/Transform.h>
#include <Game/Components/Object/ObjectChild.h>

#include <Game/Components/Object/Name.h>
#include <Debug/DebugUI.h>

namespace ecs {
	/**
	 * @brief コンストラクタ
	 * @param _desc
	 */
	TransformSystem::TransformSystem(const SystemDesc& _desc)
		: ISystem(_desc)
	{
		// デバッグ用ウィンドウを登録（エディタビルド時のみ実行）
		debug::DebugUI::ResistDebugFunction([this]() { RenderTransformHierarchy(); });

	}

	/**
	 * @brief 初期化
	 */
	void TransformSystem::Init()
	{
		// 必須コンポーネント
		Signature signature;
		signature.set(ecs_.GetComponentType<Transform>());
		ecs_.SetSystemSignature<TransformSystem>(signature);
	}

	/**
	 * @brief 更新処理
	 * @param _dt デルタタイム
	 */
	void TransformSystem::Update(float _dt)
	{
		using namespace DirectX;

		// 訪問情報
		std::unordered_set<Entity> visited;
		std::unordered_set<Entity> visiting;

		// ワールド行列更新再帰関数
		std::function<void(Entity)> updateWorld = [&](Entity e)
			{
				if (visited.count(e)) return;

				if (visiting.count(e)) {
					GameLogFError(
						"[TransformSystem] 親子関係にサイクルを検出。Entity ID: {}",
						e.id_
					);
					return;
				}

				visiting.insert(e);

				auto& tf = ecs_.GetComponent<Transform>(e);

				bool hasParent = false;
				XMMATRIX parentWorld = XMMatrixIdentity();

				// 親がいるなら先に親を更新
				if (ecs_.HasComponent<ObjectChild>(e)) {
					const auto& child = ecs_.GetComponent<ObjectChild>(e);
					if (child.root.IsInitialized() &&
						ecs_.HasComponent<Transform>(child.root))
					{
						hasParent = true;
						updateWorld(child.root);

						const auto& parentTf =
							ecs_.GetComponent<Transform>(child.root);
						parentWorld = XMLoadFloat4x4(&parentTf.world);
					}
				}

				if (tf.dirty || hasParent) {
					XMMATRIX S = XMMatrixScaling(tf.scale.x, tf.scale.y, tf.scale.z);
					XMMATRIX R = XMMatrixRotationQuaternion(
						XMLoadFloat4(&tf.rotationQuat)
					);
					XMMATRIX T = XMMatrixTranslation(
						tf.position.x, tf.position.y, tf.position.z
					);

					XMMATRIX local = S * R * T;
					XMMATRIX world = hasParent
						? XMMatrixMultiply(local, parentWorld)
						: local;

					XMStoreFloat4x4(&tf.world, world);
					tf.dirty = false;
				}

				visited.insert(e);
				visiting.erase(e);
			};


		// 全エンティティ更新
		for (auto e : entities_) {
			updateWorld(e);
		}
	}


	// ---------- デバッグ: 親子階層出力 ---------- //
	void TransformSystem::RenderTransformHierarchy()
	{
		// ImGuiウィンドウを作成
		ImGui::Begin("Transform Hierarchy");

		// 親->子マップ構築と親を持つフラグ
		std::unordered_map<Entity, std::vector<Entity>> childrenMap;
		std::unordered_set<Entity> hasParent;

		for (auto e : entities_) {
			if (!e.IsInitialized()) { continue; }

			if (ecs_.HasComponent<ObjectChild>(e)) {
				const auto& child = ecs_.GetComponent<ObjectChild>(e);
				if (child.root.IsInitialized()) {
					// 親が存在し、かつその親がこのシステムで扱われていれば登録
					// 親が Transform を持つことが前提
					if (ecs_.HasComponent<Transform>(child.root)) {
						childrenMap[child.root].push_back(e);
						hasParent.insert(e);
					}
				}
			}

			// Ensure the node exists in map so that leafs are present
			if (childrenMap.find(e) == childrenMap.end()) {
				childrenMap.emplace(e, std::vector<Entity>{});
			}
		}

		// ルートノードを収集（親を持たないエンティティ）
		std::vector<Entity> roots;
		for (auto& kv : childrenMap) {
			if (hasParent.count(kv.first) == 0) {
				roots.push_back(kv.first);
			}
		}

		// ツリー描画用の再帰関数（サイクル検出付き）
		std::unordered_set<Entity> visiting;
		std::function<void(Entity)> drawNode;
		drawNode = [&](Entity e)
			{
				// 表示ラベルを決定（Nameコンポーネントがあればそれを使う）
				std::string label;
				if (ecs_.HasComponent<Name>(e)) {
					label = ecs_.GetComponent<Name>(e).value;
				}
				else {
					label = "Idx:" + std::to_string(e.Index()) + " Ver:" + std::to_string(e.Version());
				}

				// サイクル検出
				if (visiting.count(e)) {
					ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s (Cycle detected)", label.c_str());
					return;
				}

				visiting.insert(e);

				auto it = childrenMap.find(e);
				bool hasChildren = (it != childrenMap.end() && !it->second.empty());

				ImGui::PushID(static_cast<int>(e.Index()));
				if (hasChildren) {
					// ツリーとして表示
					if (ImGui::TreeNode(label.c_str())) {
						for (auto& c : it->second) {
							drawNode(c);
						}
						ImGui::TreePop();
					}
				}
				else {
					// 葉ノードは箇条書きで表示
					ImGui::BulletText("%s", label.c_str());
				}
				ImGui::PopID();

				visiting.erase(e);
			};

		// ルートを表示（ルートが無い場合は全エンティティを一覧）
		if (roots.empty()) {
			ImGui::TextDisabled("No root nodes found. Listing all entities:");
			for (auto& kv : childrenMap) {
				drawNode(kv.first);
			}
		}
		else {
			for (auto& r : roots) {
				drawNode(r);
			}
		}

		ImGui::End();
	}

}


