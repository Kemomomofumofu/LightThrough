#pragma once
/**
 * @file CollisionUtils.h
 * @brief 当たり判定のヘルパー的関数
 * @author Arima Keita
 * @date 2025-10-03
 */

 // ---------- インクルード ---------- //
#include <cmath>
#include <algorithm>
#include <optional>
#include <variant>
#include <DirectXMath.h>
#include <Game/Serialization/ComponentReflection.h>

namespace collision {
	using namespace DirectX;

	/**
	 * @brief 形状種類
	 */
	enum class ShapeType : uint8_t {
		Sphere,
		Box,
		Max,
	};

	// ---------- ローカル形状 ---------- // 
	struct SphereShape {
		float radius{ 0.5f };
	};
	struct BoxShape {
		XMFLOAT3 halfExtents{ 0.5f, 0.5f, 0.5f };
	};

	using ShapeVariant = std::variant<SphereShape, BoxShape>;

	/**
	 * @brief OBBのローカル情報
	 */
	struct WorldSphere {
		XMFLOAT3 center{};
		float radius{};
	};

	/**
	 * @brief OBBのワールド情報
	 */
	struct WorldOBB {
		XMFLOAT3 center{};	// 中心
		XMFLOAT3 axis[3]{ {1, 0, 0},{0, 1, 0}, {0, 0, 1} };	// 各軸
		XMFLOAT3 half{};	// 半径
	};

	/**
	 * @brief OBB同士の当たり判定
	 */
	struct ContactResult {
		XMFLOAT3 normal{};	// AからBへの法線
		float penetration{};	// 衝突深度
	};


	/**
	 * @brief 減算
	 * @param _a ベクトルA
	 * @param _b ベクトルB
	 * @return 差
	 */
	inline XMFLOAT3 Sub(const XMFLOAT3& _a, const XMFLOAT3& _b)
	{
		return { _a.x - _b.x, _a.y - _b.y, _a.z - _b.z };
	}

	/**
	 * @brief 内積
	 * @param _a ベクトルA
	 * @param _b ベクトルB
	 * @return 内積
	 */
	inline float Dot(const XMFLOAT3& _a, const XMFLOAT3& _b)
	{
		return (_a.x * _b.x) + (_a.y * _b.y) + (_a.z * _b.z);
	}

	/**
	 * @brief ベクトルの長さ
	 * @param _v ベクトル
	 * @return 長さ
	 */
	inline float LengthSq(const XMFLOAT3& _v)
	{
		return Dot(_v, _v);
	}

	/**
	 * @brief ベクトルの長さ
	 * @param _v ベクトル
	 * @return 長さ
	 */
	inline float Length(const XMFLOAT3& _v)
	{
		return std::sqrt(LengthSq(_v));
	}


	/**
	 * @brief ベクトルの投影長を計算
	 * @param _b OBB
	 * @param _axis 投影する軸
	 * @return 投影長
	 */
	inline float ProjectRadius(const WorldOBB& _b, const XMFLOAT3& _axis)
	{
		return std::fabs(Dot(_b.axis[0], _axis)) * _b.half.x +
			std::fabs(Dot(_b.axis[1], _axis)) * _b.half.y +
			std::fabs(Dot(_b.axis[2], _axis)) * _b.half.z;
	}

	/**
	 * @brief ベクトルの正規化
	 * @param _v ベクトル
	 * @return 正規化されたベクトル
	 */
	inline XMFLOAT3 Normalize(const XMFLOAT3& _v)
	{
		float len = Length(_v);
		// 長さがほとんど0なら0ベクトルを返す
		if (len < 1e-6f) { return{ 0, 0, 0 }; }

		return { _v.x / len, _v.y / len, _v.z / len };
	}

	// ---------- 衝突判定関数 ---------- //
	/**
	 * @brief Sphere vs Sphere
	 * @param _sphereA
	 * @param _sphereB
	 * @return 衝突している: 衝突情報, していない: std::nullopt
	 */
	inline std::optional<ContactResult> IntersectSphere(const WorldSphere& _sphereA, const WorldSphere& _sphereB)
	{
		XMFLOAT3 d = Sub(_sphereB.center, _sphereA.center);
		float r = _sphereA.radius + _sphereB.radius;
		float distSq = LengthSq(d);
		// 当たっていない場合
		if (distSq > r * r) { return std::nullopt; }

		// 距離の計算 
		float dist = std::sqrt((std::max)(distSq, 1e-12f));	// ゼロ除算対策

		XMFLOAT3 n{};
		// 2つの球の中心間の距離がほとんど0の場合
		if (dist < 1e-6f) { n = { 0, 1, 0 }; }
		// そうでなければ正規化
		else { n = { d.x / dist, d.y / dist, d.z / dist }; }

		return ContactResult{ n, r - dist };
	}

	inline std::optional<ContactResult> IntersectSphereOBB(const WorldSphere& _sphere, const WorldOBB& _obb)
	{
		XMFLOAT3 d = Sub(_sphere.center, _obb.center);
		// OBBの各軸に投影
		float lx = Dot(d, _obb.axis[0]);
		float ly = Dot(d, _obb.axis[1]);
		float lz = Dot(d, _obb.axis[2]);
		// OBBの範囲内に収める
		lx = std::clamp(lx, -_obb.half.x, _obb.half.x);
		ly = std::clamp(ly, -_obb.half.y, _obb.half.y);
		lz = std::clamp(lz, -_obb.half.z, _obb.half.z);
		// 最近接点の計算
		XMFLOAT3 closest{
			_obb.center.x + _obb.axis[0].x * lx + _obb.axis[1].x * ly + _obb.axis[2].x * lz,
			_obb.center.y + _obb.axis[0].y * lx + _obb.axis[1].y * ly + _obb.axis[2].y * lz,
			_obb.center.z + _obb.axis[0].z * lx + _obb.axis[1].z * ly + _obb.axis[2].z * lz,
		};
		// 最近接点と球の中心の距離
		XMFLOAT3 sep = Sub(_sphere.center, closest);
		float distSq = LengthSq(sep);
		if (distSq > _sphere.radius * _sphere.radius) { return std::nullopt; }
		float dist = std::sqrt((std::max)(distSq, 1e-12f));

		// 法線と貫通深度の計算
		XMFLOAT3 n{};
		float penetration = 0;
		// 2つの球の中心間の距離がほとんど0のとき
		if (dist < 1e-6f) {
			n = { 0, 1, 0 };
			penetration = _sphere.radius;
		}
		// そうでなければ
		else {
			n = { sep.x / dist, sep.y / dist, sep.z / dist };
			penetration = _sphere.radius - dist;
		}

		return ContactResult{ n, penetration };
	}
	inline std::optional<ContactResult> IntersectOBBSphere(const WorldOBB& _obb, const WorldSphere& _sphere)
	{
		auto result = IntersectSphereOBB(_sphere, _obb);
		if (!result) { return std::nullopt; }

		// 法線を反転
		result->normal.x *= -1.0f;
		result->normal.y *= -1.0f;
		result->normal.z *= -1.0f;

		return result;
	}

	/**
	 * @brief OBB vs OBB
	 * @param _obbA
	 * @param _obbB
	 * @return 衝突している: 衝突情報, していない: std::nullopt
	 */
	inline std::optional<ContactResult> IntersectOBB(const WorldOBB& _obbA, const WorldOBB& _obbB)
	{
		constexpr float EPS = 1e-6f;	// 誤差範囲
		XMFLOAT3 T = Sub(_obbB.center, _obbA.center); // AからBへのベクトル

		XMFLOAT3 axes[15]{}; // テストする軸
		int axisCount = 0;
		for (int i = 0; i < 3; ++i) {
			axes[axisCount++] = _obbA.axis[i];
		}
		for (int i = 0; i < 3; ++i) {
			axes[axisCount++] = _obbB.axis[i];
		}

		// 3x3の外積を計算して追加
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				XMFLOAT3 cr{
					_obbA.axis[i].x * _obbB.axis[j].z - _obbA.axis[i].z * _obbB.axis[j].y,
					_obbA.axis[i].y * _obbB.axis[j].x - _obbA.axis[i].x * _obbB.axis[j].z,
					_obbA.axis[i].z * _obbB.axis[j].y - _obbA.axis[i].y * _obbB.axis[j].x
				};
				float len2 = Dot(cr, cr);
				// 誤差範囲より大きければ
				if (len2 > EPS) {
					float inv = 1.0f / std::sqrt(len2);
					axes[axisCount++] = { cr.x * inv, cr.y * inv, cr.z * inv };
				}
			}
		}

		// 最小の貫通深度とその軸
		float minPenetration = FLT_MAX;
		XMFLOAT3 bestAxis{};

		// 各軸でテスト
		for (int i = 0; i < axisCount; ++i) {
			const auto& axis = axes[i];

			if (std::fabs(axis.x) + std::fabs(axis.y) + std::fabs(axis.z) < EPS) { continue; }	// ゼロベクトルはスキップ

			// 投影
			float rA = ProjectRadius(_obbA, axis);
			float rB = ProjectRadius(_obbB, axis);
			// 投射軸上での中心距離
			float dist = std::fabs(Dot(T, axis));
			float overlap = rA + rB - dist;

			// 分離している
			if (overlap < 0.0f) {
				return std::nullopt;
			}

			// 最小の貫通深度を更新
			if (overlap < minPenetration) {
				minPenetration = overlap;
				bestAxis = axis;

				// 内積が負なら
				if (Dot(bestAxis, T) < 0.0f) {
					// 中心方向に合わせて法線を反転
					bestAxis.x *= -1.0f;
					bestAxis.y *= -1.0f;
					bestAxis.z *= -1.0f;
				}
			}
		}

		return ContactResult{ bestAxis, minPenetration };
	}



	/**
	 * @brief 貫通解消量の計算
	 * @param _contact		A->Bへの法線と貫通震度
	 * @param _isStaticA	Aが静的か
	 * @param _isStaticB	Bが静的か
	 * @param _percent		解消割合(過補正対策 推奨は0.8f程度)
	 * @param _slop			微小貫通を無視する閾値(推奨は0.01f程度)
	 * @return {Aの移動量, Bの移動量} それぞれのTransformに加算するオフセット
	 */
	inline std::pair<XMFLOAT3, XMFLOAT3> ComputePushOut(
		const ContactResult& _contact,
		bool _isStaticA, bool _isStaticB,
		float _percent = 0.8f, float _slop = 0.01f)
	{
		float corr = (std::max)(_contact.penetration - _slop, 0.0f) * _percent;

		XMFLOAT3 n = Normalize(_contact.normal);
		XMFLOAT3 dispA{ 0, 0, 0 };
		XMFLOAT3 dispB{ 0, 0, 0 };

		// どちらも静的
		if (_isStaticA && _isStaticB) {
			return { dispA, dispB };
		}
		// Aだけ静的
		else if (_isStaticA && !_isStaticB) {
			dispB = { n.x * corr, n.y * corr, n.z * corr };
		}
		// Bだけ静的
		else if (!_isStaticA && _isStaticB) {
			dispA = { -n.x * corr, -n.y * corr, -n.z * corr };
		}
		// どちらも動的
		else {
			// 半分ずつ移動
			float halfCorr = corr * 0.5f;
			dispA = { -n.x * halfCorr, -n.y * halfCorr, -n.z * halfCorr };
			dispB = { n.x * halfCorr, n.y * halfCorr, n.z * halfCorr };
		}

		return { dispA, dispB };
	}

	inline constexpr const char* ToString(ShapeType _t)
	{
		switch (_t)
		{
		case ShapeType::Sphere: return "Sphere";
		case ShapeType::Box:	return "Box";
		default:				return "Unknown";
		}
	}

	inline std::optional<ShapeType> ShapeTypeFromString(std::string_view _s)
	{
		if (_s == "Sphere") { return ShapeType::Sphere; }
		if (_s == "Box") { return ShapeType::Box; }
		return std::nullopt;
	}

} // namespace collision

ECS_REFLECT_BEGIN(collision::SphereShape)
ECS_REFLECT_FIELD(radius)
ECS_REFLECT_END()

ECS_REFLECT_BEGIN(collision::BoxShape)
ECS_REFLECT_FIELD(halfExtents)
ECS_REFLECT_END()



// ShapeVariant 用のSerialize/Deserialize
namespace ecs_serial
{
	template<>
	inline json Serialize<collision::ShapeVariant>(const collision::ShapeVariant& _v)
	{
		json j = json::object();
		std::visit([&](auto&& _s) {
			using S = std::decay_t<decltype(_s)>;
			// SphereShape
			if constexpr (std::is_same_v<S, collision::SphereShape>) {
				j["type"] = collision::ToString(collision::ShapeType::Sphere);
				j["data"] = Serialize(_s);
			}
			// BoxShape
			else if constexpr (std::is_same_v<S, collision::BoxShape>) {
				j["type"] = collision::ToString(collision::ShapeType::Box);
				j["data"] = Serialize(_s);
			}
		}, _v);
		return j;
	}

	template<>
	inline collision::ShapeVariant Deserialize<collision::ShapeVariant>(const json& _j)
	{
		// memo: 一応数値/文字列の両対応にしておく
		collision::ShapeType type{};
		if (_j.contains("type")) {
			if (_j["type"].is_string()) {
				auto t = collision::ShapeTypeFromString(_j["type"].get<std::string>());
				if (!t) { throw std::runtime_error("[ShapeVariant] Unknown type string"); }
				type = *t;
			}
			else {
				type = static_cast<collision::ShapeType>(_j["type"].get<std::underlying_type_t<collision::ShapeType>>());
			}
		}
		else {
			throw std::runtime_error("[ShapeVariant Missing 'type']");
		}

		const json& data = _j.contains("data") ? _j.at("data") : json::object();

		switch (type) {
		case collision::ShapeType::Sphere:
		{
			auto s = Deserialize<collision::SphereShape>(data);
			return collision::ShapeVariant{ s };
		}
		case collision::ShapeType::Box:
		{
			auto b = Deserialize<collision::BoxShape>(data);
			return collision::ShapeVariant{ b };
		}
		default:
			throw std::runtime_error("[ShapeVariant] Unsupported type");
		}
	}
} // namespace ecs_serial