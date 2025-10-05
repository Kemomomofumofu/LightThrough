#pragma once
/**
 * @file ComponentReflection.h
 * @brief コンポーネントのリフレクション
 * @author Arima Keita
 * @date 2025-10-06
 */

 /*---------- インクルード ----------*/
#include <tuple>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <cassert>
#include <nlohmann/json.hpp>
#include <Game/ECS/Coordinator.h>

namespace ecs_serial {
	using json = nlohmann::json;

	/**
	 * @brief フィールド情報
	 * @tparam T		親型(Component)
	 * @tparam MemberT	メンバ型
	 */
	template<class T, class MemberT>
	struct FieldInfo {
		std::string_view name;	// jsonキー/フィールド名
		MemberT T::* member;	// メンバポインタ
	};

	/**
	 * @brief 型のリフレクション情報
	 * @tparam T
	 */
	template<class T>
	struct TypeReflection {
		/**
		* @brief フィールド情報のタプルを返す
		*
		* デフォルトは空のタプルを返す
		*
		* @return フィールド情報のタプル
		*/
		static constexpr auto Fields()
		{
			return std::make_tuple();
		}

		/**
		 * @brief 型名を返す
		 *
		 * デフォルトは "Unknown" を返す
		 *
		 * @return 型名
		 */
		static constexpr std::string_view Name() { return "Unknown"; }
	};

	/**
	 * @brief tuple for_each ユーティリティ
	 */
	template<class Tuple, class F, std::size_t... I>
	constexpr void for_each_impl(Tuple&& _t, F&& _f, std::index_sequence<I...>)
	{
		(_f(std::get<I>(_t)), ...);
	}

	/**
	 * @brief tuple の全要素に対してラムダを適用
	 */
	template<class Tuple, class F>
	constexpr void for_each(Tuple&& _t, F&& _f)
	{
		for_each_impl(std::forward<Tuple>(_t), std::forward<F>(_f),
			std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
	}

	/**
	 * @brief std::array判定のヘルパー
	 */
	template<class T> struct is_std_array : std::false_type {};
	template<class U, std::size_t N> struct is_std_array<std::array<U, N>> : std::true_type {};
	template<class T> inline constexpr bool is_std_array_v = is_std_array<T>::value;

	/**
	 * @brief ベクトルライクな型の判定
	 *
	 * x, y, z(, w)を float に変換可能なメンバとして持つ
	 */
	template<class T>
	concept Vec3Like = requires(T _v) {
		{ _v.x } -> std::convertible_to<float>;
		{ _v.y } -> std::convertible_to<float>;
		{ _v.z } -> std::convertible_to<float>;
	};
	template<class T>
	concept Vec4Like = requires(T _v) {
		{ _v.w } -> std::convertible_to<float>;
		{ _v.x } -> std::convertible_to<float>;
		{ _v.y } -> std::convertible_to<float>;
		{ _v.z } -> std::convertible_to<float>;
	};

	/**
	 * @brief JSON値を型Tに代入
	 *
	 * 新しい型をサポートしたい場合は、
	 * `if constexpr`の連鎖に`std::is_same_v<T, YourType>`などを追加
	 *
	 * @tparam T
	 * @param _dst 代入先
	 * @param _j JSONオブジェクト
	 */
	template<class T>
	inline void assign_value(T& _dst, const json& _j)
	{
		// 算術型/bool
		if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
			_dst = _j.get<T>();
		}
		// enum/enum class
		else if constexpr (std::is_enum_v<T>) {
			using UT = std::underlying_type_t<T>;
			_dst = static_cast<T>(_j.get<UT>());
		}
		// 文字列
		else if constexpr (std::is_same_v<T, std::string>) {
			_dst = _j.get<std::string>();
		}
		// C配列
		else if constexpr (std::is_array_v<T>) {
			using Elem = std::remove_extent_t<T>;
			for (size_t i = 0; i < std::extent_v<T>; ++i) {
				assign_value(reinterpret_cast<Elem&>(_dst[i]), _j.at(i));
			}
		}
		// std::array
		else if constexpr (is_std_array_v<T>) {
			for (size_t i = 0; i < std::size(_dst); ++i) {
				assign_value(_dst[i], _j.at(i));
			}
		}
		// 要素が三つのもの
		else if constexpr (Vec3Like<T>) {
			_dst.x = _j.at(0).get<float>();
			_dst.y = _j.at(1).get<float>();
			_dst.z = _j.at(2).get<float>();
		}
		// 要素が四つのもの
		else if constexpr (Vec4Like<T>) {
			_dst.x = _j.at(0).get<float>();
			_dst.y = _j.at(1).get<float>();
			_dst.z = _j.at(2).get<float>();
			_dst.w = _j.at(3).get<float>();
		}
		// クラス型
		else if constexpr (std::is_class_v<T>) {
			_dst = Deserialize<T>(_j);
		}
		// どれでもない
		else {
			static_assert(sizeof(T) == 0, "[ComponentReflection.h] 未対応の型があります");
		}
	}

	/**
	 * @brief JSONからオブジェクトへ
	 */
	template<class T>
	T Deserialize(const json& _j)
	{
		T obj{};
		auto fields = TypeReflection<T>::Fields();
		for_each(fields, [&](auto&& f) {
			if (_j.contains(f.name)) {
				assign_value(obj.*(f.member), _j.at(f.name));
			}
			});

		return obj;
	}

	/**
	 * @brief コンポーネントレジストリ
	 *
	 * SceneJsonに記録されたComponentNameから動的にAddComponentを呼ぶ
	 */
	template<class Coordinator, class Entity>
	class ComponentRegistry {
	public:
		// AddComponent呼び出しラッパ
		using AddFunc = std::function<void(Coordinator&, Entity, const json&)>;

		/**
		 * @brief シングルトン取得
		 * @return インスタンス
		 */
		static ComponentRegistry& Get()
		{
			static ComponentRegistry inst;
			return inst;
		}

		/**
		 * @brief 名前と追加関数を登録
		 * @param _name コンポーネント名
		 * @param _f AddComponent 実行ラムダ
		 */
		void Register(std::string_view _name, AddFunc _f)
		{
			registry_[std::string(_name)] = std::move(_f);
		}

		/**
		* @brief 名前が存在すればコンポーネントを追加
		* @return 追加できた: true, できない: false
		*/
		bool AddIfExists(Coordinator& _coord, Entity _e, std::string_view _name, const json& _data)
		{
			auto it = registry_.find(std::string(_name));
			if (it == registry_.end()) { return false; }

			it->second(_coord, _e, _data);

			return true;
		}

	private:
		std::unordered_map<std::string, AddFunc> registry_;	// name -> deserializer
	};
}



// ---------- リフレクション定義マクロ ---------- // 
/**
 * @brief 型特殊化開始
 * 例:
 *   ECS_REFLECT_BEGIN(Transform)
 *     ECS_REFLECT_FIELD(position),
 *     ECS_REFLECT_FIELD(scale)
 *   ECS_REFLECT_END()
 */
#define ECS_REFLECT_BEGIN(Type) \
template<> struct ecs_serial::TypeReflection<Type> { \
	using This = Type; \
	static constexpr std::string_view Name() { return #Type; } \
	static constexpr auto Fields() { \
		return std::make_tuple(

/**
 * @brief フィールド列挙マクロ
 * 末尾カンマは呼び出し側で調整 (最後のフィールドはカンマ無し)
 */
#define ECS_REFLECT_FIELD(Member) \
		ecs_serial::FieldInfo<This, decltype(This::Member)>{ std::string_view(#Member), &This::Member }

/**
 * @brief 型特殊化終了
 */
#define ECS_REFLECT_END() ); } };

 /**
  * @brief コンポーネント用デシリアライザ登録マクロ
  *
  * CoordinatorT::AddComponent<ComponentT>(entity, component) を呼び出す
  * 事前に TypeReflection<ComponentT> が定義されている必要がある
  *
  * 呼び出しタイミング例:
  *   void RegisterAll() {
  *     REGISTER_COMPONENT_DESERIALIZER(Coordinator, Entity, Transform);
  *     REGISTER_COMPONENT_DESERIALIZER(Coordinator, Entity, Camera);
  *   }
  */
#define REGISTER_COMPONENT_DESERIALIZER(CoordinatorT, EntityT, ComponentT) \
	do { \
		ecs_serial::ComponentRegistry<CoordinatorT, EntityT>::Get().Register( \
			ecs_serial::TypeReflection<ComponentT>::Name(), \
			[](CoordinatorT& _coord, EntityT _e, const nlohmann::json& _j){ \
				const ComponentT temp = ecs_serial::Deserialize<ComponentT>(_j); \
				_coord.AddComponent<ComponentT>(_e, temp); \
			}); \
	} while(0)