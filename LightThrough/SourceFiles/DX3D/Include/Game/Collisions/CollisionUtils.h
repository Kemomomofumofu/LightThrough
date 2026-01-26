#pragma once
/**
 * @file CollisionUtils.h
 * @brief 当たり判定のヘルパー的関数
 */

 // ---------- インクルード ---------- //
#include <cmath>
#include <algorithm>
#include <optional>
#include <variant>
#include <DirectXMath.h>
#include <DX3D/Math/MathUtils.h>
#include <Game/Serialization/ComponentReflection.h>

namespace dx3d {
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

		using ShapeVariant = std::variant<BoxShape, SphereShape>;

		/**
		 * @brief 球のワールド情報
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
			XMFLOAT3 axis[3]{ {1, 0, 0},{0, 1, 0}, {0, 0, 1} };	// 各軸（ワールド空間で正規化されている前提）
			XMFLOAT3 half{};	// 半長（半幅）
		};

		/**
		 * @brief 衝突結果
		 */
		struct ContactResult {
			XMFLOAT3 normal{};       // AからBへの法線（A->B）
			float penetration{};     // 衝突深度
			XMFLOAT3 contactPoint{}; // 近似接触点（SAT由来）
		};

		/**
		 * @brief ベクトルの投影長を計算
		 */
		inline float ProjectRadius(const WorldOBB& _b, const XMFLOAT3& _axis)
		{
			return std::fabs(math::Dot(_b.axis[0], _axis)) * _b.half.x +
				std::fabs(math::Dot(_b.axis[1], _axis)) * _b.half.y +
				std::fabs(math::Dot(_b.axis[2], _axis)) * _b.half.z;
		}


		// ---------- 衝突判定関数 ---------- //
		inline std::optional<ContactResult> IntersectSphere(const WorldSphere& _sphereA, const WorldSphere& _sphereB)
		{
			XMFLOAT3 d = math::Sub(_sphereB.center, _sphereA.center);
			float r = _sphereA.radius + _sphereB.radius;
			float distSq = math::LengthSq(d);
			if (distSq > r * r) { return std::nullopt; }

			float dist = std::sqrt((std::max)(distSq, 1e-12f));
			XMFLOAT3 n{};
			if (dist < 1e-6f) { n = { 0, 1, 0 }; }
			else { n = { d.x / dist, d.y / dist, d.z / dist }; }

			XMFLOAT3 contactPoint = {
				_sphereA.center.x + n.x * _sphereA.radius,
				_sphereA.center.y + n.y * _sphereA.radius,
				_sphereA.center.z + n.z * _sphereA.radius
			};

			return ContactResult{ n, r - dist, contactPoint };
		}

		inline std::optional<ContactResult> IntersectSphereOBB(const WorldSphere& _sphere, const WorldOBB& _obb)
		{
			XMFLOAT3 d = math::Sub(_sphere.center, _obb.center);
			float lx = math::Dot(d, _obb.axis[0]);
			float ly = math::Dot(d, _obb.axis[1]);
			float lz = math::Dot(d, _obb.axis[2]);
			lx = std::clamp(lx, -_obb.half.x, _obb.half.x);
			ly = std::clamp(ly, -_obb.half.y, _obb.half.y);
			lz = std::clamp(lz, -_obb.half.z, _obb.half.z);

			XMFLOAT3 closest{
				_obb.center.x + _obb.axis[0].x * lx + _obb.axis[1].x * ly + _obb.axis[2].x * lz,
				_obb.center.y + _obb.axis[0].y * lx + _obb.axis[1].y * ly + _obb.axis[2].y * lz,
				_obb.center.z + _obb.axis[0].z * lx + _obb.axis[1].z * ly + _obb.axis[2].z * lz,
			};

			XMFLOAT3 sep = math::Sub(_sphere.center, closest);
			float distSq = math::LengthSq(sep);
			if (distSq > _sphere.radius * _sphere.radius) { return std::nullopt; }
			float dist = std::sqrt((std::max)(distSq, 1e-12f));

			XMFLOAT3 n{};
			float penetration = 0;
			if (dist < 1e-6f) {
				n = { 0, 1, 0 };
				penetration = _sphere.radius;
			}
			else {
				n = { sep.x / dist, sep.y / dist, sep.z / dist };
				penetration = _sphere.radius - dist;
			}

			return ContactResult{ n, penetration, closest };
		}

		inline std::optional<ContactResult> IntersectOBBSphere(const WorldOBB& _obb, const WorldSphere& _sphere)
		{
			auto result = IntersectSphereOBB(_sphere, _obb);
			if (!result) { return std::nullopt; }

			result->normal.x *= -1.0f;
			result->normal.y *= -1.0f;
			result->normal.z *= -1.0f;

			return result;
		}

		// OBB vs OBB（SAT ベース）
		inline std::optional<ContactResult> IntersectOBB(const WorldOBB& _obbA, const WorldOBB& _obbB)
		{
			constexpr float EPS = 1e-6f;
			XMFLOAT3 T = math::Sub(_obbB.center, _obbA.center);

			XMFLOAT3 axes[15]{};
			int axisCount = 0;
			for (int i = 0; i < 3; ++i) axes[axisCount++] = _obbA.axis[i];
			for (int i = 0; i < 3; ++i) axes[axisCount++] = _obbB.axis[i];

			for (int i = 0; i < 3; ++i) {
				for (int j = 0; j < 3; ++j) {
					XMFLOAT3 cr = math::Cross(_obbA.axis[i], _obbB.axis[j]);
					float len2 = math::Dot(cr, cr);
					if (len2 > EPS) {
						float inv = 1.0f / std::sqrt(len2);
						axes[axisCount++] = math::Scale(cr, inv);
					}
				}
			}

			float minPenetration = FLT_MAX;
			XMFLOAT3 bestAxis{};

			for (int i = 0; i < axisCount; ++i) {
				const auto& axis = axes[i];
				if (std::fabs(axis.x) + std::fabs(axis.y) + std::fabs(axis.z) < EPS) { continue; }

				float rA = ProjectRadius(_obbA, axis);
				float rB = ProjectRadius(_obbB, axis);
				float dist = std::fabs(math::Dot(T, axis));
				float overlap = rA + rB - dist;

				if (overlap < 0.0f) {
					return std::nullopt;
				}

				if (overlap < minPenetration) {
					minPenetration = overlap;
					bestAxis = axis;
					if (math::Dot(axis, T) < 0.0f) {
						bestAxis.x *= -1.0f;
						bestAxis.y *= -1.0f;
						bestAxis.z *= -1.0f;
					}
				}
			}

			// 最近接点中点法（保持しておく）
			XMFLOAT3 d = math::Sub(_obbB.center, _obbA.center);
			float lx = math::Dot(d, _obbA.axis[0]);
			float ly = math::Dot(d, _obbA.axis[1]);
			float lz = math::Dot(d, _obbA.axis[2]);
			lx = std::clamp(lx, -_obbA.half.x, _obbA.half.x);
			ly = std::clamp(ly, -_obbA.half.y, _obbA.half.y);
			lz = std::clamp(lz, -_obbA.half.z, _obbA.half.z);

			XMFLOAT3 closestOnA{
				_obbA.center.x + _obbA.axis[0].x * lx + _obbA.axis[1].x * ly + _obbA.axis[2].x * lz,
				_obbA.center.y + _obbA.axis[0].y * lx + _obbA.axis[1].y * ly + _obbA.axis[2].y * lz,
				_obbA.center.z + _obbA.axis[0].z * lx + _obbA.axis[1].z * ly + _obbA.axis[2].z * lz,
			};

			XMFLOAT3 d2 = math::Sub(_obbA.center, _obbB.center);
			float lx2 = math::Dot(d2, _obbB.axis[0]);
			float ly2 = math::Dot(d2, _obbB.axis[1]);
			float lz2 = math::Dot(d2, _obbB.axis[2]);
			lx2 = std::clamp(lx2, -_obbB.half.x, _obbB.half.x);
			ly2 = std::clamp(ly2, -_obbB.half.y, _obbB.half.y);
			lz2 = std::clamp(lz2, -_obbB.half.z, _obbB.half.z);

			XMFLOAT3 closestOnB{
				_obbB.center.x + _obbB.axis[0].x * lx2 + _obbB.axis[1].x * ly2 + _obbB.axis[2].x * lz2,
				_obbB.center.y + _obbB.axis[0].y * lx2 + _obbB.axis[1].y * ly2 + _obbB.axis[2].y * lz2,
				_obbB.center.z + _obbB.axis[0].z * lx2 + _obbB.axis[1].z * ly2 + _obbB.axis[2].z * lz2,
			};

			XMFLOAT3 contactPoint = {
				(closestOnA.x + closestOnB.x) * 0.5f,
				(closestOnA.y + closestOnB.y) * 0.5f,
				(closestOnA.z + closestOnB.z) * 0.5f
			};

			return ContactResult{ bestAxis, minPenetration, contactPoint };
		}


		/**
		 * @brief OBB上の代表接触点を取得
		 * @param target: 代表点を取る対象のOBB
		 * @param normal: 接触法線（もう一方のOBBから見た方向）
		 */
		inline XMFLOAT3 GetRepresentativeContactPointOnOBB(
			const WorldOBB& target,
			const XMFLOAT3& normal)
		{
			// normal を正規化
			XMFLOAT3 n = math::Normalize(normal);

			// 各軸との内積
			float dx = math::Dot(n, target.axis[0]);
			float dy = math::Dot(n, target.axis[1]);
			float dz = math::Dot(n, target.axis[2]);

			float adx = std::fabs(dx);
			float ady = std::fabs(dy);
			float adz = std::fabs(dz);

			int idx = 0;
			if (ady >= adx && ady >= adz) idx = 1;
			else if (adz >= adx) idx = 2;

			float sign = ((idx == 0 ? dx : (idx == 1 ? dy : dz)) > 0.0f) ? 1.0f : -1.0f;
			float half = (idx == 0) ? target.half.x : (idx == 1 ? target.half.y : target.half.z);

			return {
				target.center.x + target.axis[idx].x * sign * half,
				target.center.y + target.axis[idx].y * sign * half,
				target.center.z + target.axis[idx].z * sign * half
			};
		}

		// ComputePushOut, GetOBBCorners, GetSphereSamplePoints はそのまま
		inline std::pair<XMFLOAT3, XMFLOAT3> ComputePushOut(
			const ContactResult& _contact,
			bool _isStaticA, bool _isStaticB,
			float _percent = 0.8f, float _slop = 0.01f)
		{
			float corr = (std::max)(_contact.penetration - _slop, 0.0f) * _percent;

			XMFLOAT3 n = math::Normalize(_contact.normal);
			XMFLOAT3 dispA{ 0, 0, 0 };
			XMFLOAT3 dispB{ 0, 0, 0 };

			if (_isStaticA && _isStaticB) {
				return { dispA, dispB };
			}
			else if (_isStaticA && !_isStaticB) {
				dispB = { n.x * corr, n.y * corr, n.z * corr };
			}
			else if (!_isStaticA && _isStaticB) {
				dispA = { -n.x * corr, -n.y * corr, -n.z * corr };
			}
			else {
				float halfCorr = corr * 0.5f;
				dispA = { -n.x * halfCorr, -n.y * halfCorr, -n.z * halfCorr };
				dispB = { n.x * halfCorr, n.y * halfCorr, n.z * halfCorr };
			}

			return { dispA, dispB };
		}

		inline void GetOBBCorners(const WorldOBB& _obb, XMFLOAT3 _outCorners[8])
		{
			const auto& c = _obb.center;
			const auto& ax = _obb.axis;
			const auto& h = _obb.half;

			XMFLOAT3 extX = math::Scale(_obb.axis[0], _obb.half.x);
			XMFLOAT3 extY = math::Scale(_obb.axis[1], _obb.half.y);
			XMFLOAT3 extZ = math::Scale(_obb.axis[2], _obb.half.z);

			_outCorners[0] = math::Sub(math::Sub(math::Sub(c, extX), extY), extZ);
			_outCorners[1] = math::Sub(math::Sub(math::Add(c, extX), extY), extZ);
			_outCorners[2] = math::Sub(math::Add(math::Sub(c, extX), extY), extZ);
			_outCorners[3] = math::Sub(math::Add(math::Add(c, extX), extY), extZ);
			_outCorners[4] = math::Add(math::Sub(math::Sub(c, extX), extY), extZ);
			_outCorners[5] = math::Add(math::Sub(math::Add(c, extX), extY), extZ);
			_outCorners[6] = math::Add(math::Add(math::Sub(c, extX), extY), extZ);
			_outCorners[7] = math::Add(math::Add(math::Add(c, extX), extY), extZ);
		}

		inline void GetSphereSamplePoints(const WorldSphere& _sphere, std::vector<XMFLOAT3>& _outPoints, bool _includeAxes = false)
		{
			_outPoints.push_back(_sphere.center);

			if (_includeAxes) {
				float r = _sphere.radius;
				_outPoints.push_back({ _sphere.center.x + r, _sphere.center.y, _sphere.center.z });
				_outPoints.push_back({ _sphere.center.x - r, _sphere.center.y, _sphere.center.z });
				_outPoints.push_back({ _sphere.center.x, _sphere.center.y + r, _sphere.center.z });
				_outPoints.push_back({ _sphere.center.x, _sphere.center.y - r, _sphere.center.z });
				_outPoints.push_back({ _sphere.center.x, _sphere.center.y, _sphere.center.z + r });
				_outPoints.push_back({ _sphere.center.x, _sphere.center.y, _sphere.center.z - r });
			}
		}

		/**
		 * @brief OBB同士の重なり領域からサンプルポイントを生成
		 * @details 代表点は小さい方OBB上の面に作り、その面を中心にグリッドサンプリングする
		 */
		inline void GenerateOverlapSamplePoints(
			const WorldOBB& _targetOBB,
			const XMFLOAT3& _contactNormal,
			std::vector<XMFLOAT3>& _outPoints,
			int _samplesPerAxis = 3
		)
		{
			XMFLOAT3 normal = math::Normalize(_contactNormal);

			// 代表接触点を取得
			XMFLOAT3 center = GetRepresentativeContactPointOnOBB(_targetOBB, normal);

			// サンプリング範囲の決定
			float minHalf = (std::min)({
					_targetOBB.half.x,
					_targetOBB.half.y,
					_targetOBB.half.z
				});
			float sampleRadius = minHalf * 0.8f;

			// 法線に垂直な基底を作りグリッド状にサンプリング
			XMFLOAT3 tangent1{}, tangent2{};
			if (std::fabs(normal.y) < 0.99f) {
				XMFLOAT3 up{ 0, 1, 0 };
				tangent1 = math::Cross(normal, up);
			}
			else {
				XMFLOAT3 right{ 1, 0, 0 };
				tangent1 = math::Cross(normal, right);
			}
			tangent1 = math::Normalize(tangent1);
			tangent2 = math::Normalize(math::Cross(normal, tangent1));

			float step = (_samplesPerAxis > 1) ? (2.0f * sampleRadius / (_samplesPerAxis - 1)) : 0.0f;
			float startOffset = -sampleRadius;

			for (int i = 0; i < _samplesPerAxis; ++i) {
				for (int j = 0; j < _samplesPerAxis; ++j) {
					float offsetU = (_samplesPerAxis > 1) ? (startOffset + step * i) : 0.0f;
					float offsetV = (_samplesPerAxis > 1) ? (startOffset + step * j) : 0.0f;

					XMFLOAT3 samplePoint{
						center.x + tangent1.x * offsetU + tangent2.x * offsetV,
						center.y + tangent1.y * offsetU + tangent2.y * offsetV,
						center.z + tangent1.z * offsetU + tangent2.z * offsetV
					};

					_outPoints.push_back(samplePoint);
				}
			}
		}
	} // namespace collision
} // namespace dx3d

namespace collision = dx3d::collision;


ECS_REFLECT_BEGIN(collision::BoxShape)
ECS_REFLECT_FIELD(halfExtents)
ECS_REFLECT_END()

ECS_REFLECT_BEGIN(collision::SphereShape)
ECS_REFLECT_FIELD(radius)
ECS_REFLECT_END()
