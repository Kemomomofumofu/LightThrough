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
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <array>
#include <nlohmann/json.hpp>


#include <Debug/Debug.h>

namespace ecs {
	struct Entity;
	class Coordinator;
}

namespace ecs_serial {
	using json = nlohmann::json;

	/**
	 * @brief フィールド情報
	 * @tparam T		親型(Component)
	 * @tparam MemberT	メンバ型
	 */
	template<class T, class MemberT>
	struct FieldInfo
	{
		std::string_view name;	// jsonキー/フィールド名
		MemberT T::* member;	// メンバポインタ
	};

	/**
	 * @brief 型のリフレクション情報
	 * @tparam T
	 */
	template<class T>
	struct TypeReflection
	{
		static constexpr auto Fields() { return std::make_tuple(); }
		static constexpr std::string_view Name() { return "Unknown"; }
	};


	/**
	 * @brief コンポーネントレジストリクラス
	 *
	 * SceneJsonに記録されたComponentNameから動的にAddComponentを呼ぶ
	 */
	class ComponentRegistry
	{
	public:
		// 呼び出しラッパ
		using AddFunc = std::function<void(ecs::Coordinator&, ecs::Entity&, const json&)>;
		using HasFunc = std::function<bool(ecs::Coordinator&, ecs::Entity&)>;
		using ToJsonFunc = std::function<json(ecs::Coordinator&, ecs::Entity&)>;

		struct Entry
		{
			AddFunc add{};
			HasFunc has{};
			ToJsonFunc toJson{};
		};

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
		 * @param _add AddComponent 実行ラムダ
		 * @param _has HasComponent 実行ラムダ
		 * @param _toJson ToJson 実行ラムダ
		 */
		void Register(std::string_view _name, AddFunc _add, HasFunc _has, ToJsonFunc _toJson)
		{
			assert(registry_.find(std::string(_name)) == registry_.end());
			registry_[std::string(_name)] = Entry{std::move(_add), std::move(_has), std::move(_toJson)};
		}

		/**
		* @brief 名前が存在すればコンポーネントを追加
		* @return 追加できた: true, できない: false
		*/
		[[nodiscard]] bool AddIfExists(ecs::Coordinator& _coord, ecs::Entity& _e, std::string_view _name, const json& _data)
		{
			auto it = registry_.find(std::string(_name));
			if (it == registry_.end()) {
				DebugLogWarning("[ComponentReflection] 存在しないComponent: '{}'", _name);
				return false;
			}

			it->second.add(_coord, _e, _data);

			return true;
		}

		json SerializeComponents(ecs::Coordinator& _coord, ecs::Entity& _e)
		{
			json comps = json::object();
			for (auto& [name, entry] : registry_) {
				if (entry.has && entry.toJson && entry.has(_coord, _e)) {
					comps[name] = entry.toJson(_coord, _e);
				}
			}
			return comps;
		}

	private:
		std::unordered_map<std::string, Entry> registry_;	// name -> entry
	};


	template<class T>
	T Deserialize(const json& _j);
	template<class T>
	json Serialize(const T& _src);


	// tuple for_each ヘルパー
	template<class Tuple, class F, std::size_t... I>
	constexpr void for_each_impl(Tuple&& _t, F&& _f, std::index_sequence<I...>)
	{
		(_f(std::get<I>(_t)), ...);
	}

	template<class Tuple, class F>
	constexpr void for_each(Tuple&& _t, F&& _f)
	{
		for_each_impl(std::forward<Tuple>(_t), std::forward<F>(_f),
			std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
	}

	// std::array 判定ヘルパー
	template<class T> struct is_std_array : std::false_type {};
	template<class U, std::size_t N> struct is_std_array<std::array<U, N>> : std::true_type {};
	template<class T> inline constexpr bool is_std_array_v = is_std_array<T>::value;


	// Vec3, Vec4 判定ヘルパー
	template<class T>
	concept Vec3Like = requires(T _v)
	{
		{ _v.x } -> std::convertible_to<float>;
		{ _v.y } -> std::convertible_to<float>;
		{ _v.z } -> std::convertible_to<float>;
	};
	template<class T>
	concept Vec4Like = requires(T _v)
	{
		{ _v.w } -> std::convertible_to<float>;
		{ _v.x } -> std::convertible_to<float>;
		{ _v.y } -> std::convertible_to<float>;
		{ _v.z } -> std::convertible_to<float>;
	};

	// static_assert 用ヘルパー
	template<class> inline constexpr bool always_false_v = false;

	/**
	 * @brief JSONから型Tに代入(Deserialize)
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
				assign_value(_dst[i], _j.at(i));
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
			if (_j.is_object()) {
				_dst.x = _j.value("x", 0.0f);
				_dst.y = _j.value("y", 0.0f);
				_dst.z = _j.value("z", 0.0f);
			}
			else {
				_dst.x = _j.size() > 0 ? _j.at(0).get<float>() : 0.0f;
				_dst.y = _j.size() > 1 ? _j.at(1).get<float>() : 0.0f;
				_dst.z = _j.size() > 2 ? _j.at(2).get<float>() : 0.0f;
			}
		}
		// 要素が四つのもの
		else if constexpr (Vec4Like<T>) {
			if (_j.is_object()) {
				_dst.x = _j.value("x", 0.0f);
				_dst.y = _j.value("y", 0.0f);
				_dst.z = _j.value("z", 0.0f);
				_dst.w = _j.value("w", 1.0f);

			}
			else {
				_dst.x = _j.size() > 0 ? _j.at(0).get<float>() : 0.0f;
				_dst.y = _j.size() > 1 ? _j.at(1).get<float>() : 0.0f;
				_dst.z = _j.size() > 2 ? _j.at(2).get<float>() : 0.0f;
				_dst.w = _j.size() > 3 ? _j.at(3).get<float>() : 1.0f;
			}
		}
		// クラス型
		else if constexpr (std::is_class_v<T>) {
			_dst = Deserialize<T>(_j);
		}
		// どれでもない
		else {
			static_assert(always_false_v<T>, "[ComponentReflection.h] 未対応の型があります");
		}
	}

	// シリアライズで使う
	template<class T>
	inline json to_json_value(const T& _src)
	{
		// 算術型/bool
		if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, bool>) {
			return json(_src);
		}
		// enum
		else if constexpr (std::is_enum_v<T>) {
			return static_cast<std::underlying_type_t<T>>(_src);
		}
		// 文字列
		else if constexpr (std::is_same_v<T, std::string>) {
			return json(_src);
		}
		// C配列
		else if constexpr (std::is_array_v<T>) {
			json arr = json::array();
			for (size_t i = 0; i < std::extent_v<T>; ++i) {
				arr.push_back(to_json_value(_src[i]));
			}
			return arr;
		}
		// std::array
		else if constexpr (is_std_array_v<T>) {
			json arr = json::array();
			for (const auto& e : _src) {
				arr.push_back(to_json_value(e));
			}
			return arr;
		}
		// 要素が三つのもの
		else if constexpr (Vec3Like<T>) {
			return json{ {"x", _src.x}, {"y", _src.y}, {"z", _src.z} };
		}
		// 要素が四つのもの
		else if constexpr (Vec4Like<T>) {
			return json{ {"x", _src.x}, {"y", _src.y}, {"z", _src.z}, {"w", _src.w} };
		}
		// クラス型
		else if constexpr (std::is_class_v<T>) {
			return Serialize(_src);
		}
		else {
			static_assert(always_false_v<T>, "[ComponentReflection.h] 未対応の型があります");
		}
	}

	/**
	 * @brief オブジェクト -> JSON
	 */
	template<class T>
	json Serialize(const T& _obj)
	{
		json j = json::object();
		auto fields = TypeReflection<T>::Fields();
		for_each(fields, [&](auto&& _f) {
			const auto& value = _obj.*(_f.member);
			j[_f.name] = to_json_value(value);
			});

		return j;
	}

	/**
	 * @brief JSON -> オブジェクト
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

} // namespace ecs_serial



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
 * @brief コンポーネントのリフレクション登録マクロ
 * @tparam ComponentT	コンポーネント型
 */
#define REGISTER_COMPONENT_REFLECTION(ComponentT) \
	do { \
		ecs_serial::ComponentRegistry::Get().Register( \
			ecs_serial::TypeReflection<ComponentT>::Name(), \
			[](ecs::Coordinator& _coord, ecs::Entity _e, const nlohmann::json& _j){ \
				const ComponentT temp = ecs_serial::Deserialize<ComponentT>(_j); \
				_coord.AddComponent<ComponentT>(_e, temp); \
			}, \
			[](ecs::Coordinator& _coord, ecs::Entity _e) { \
				return _coord.HasComponent<ComponentT>(_e); \
			 }, \
			[](ecs::Coordinator& _coord, ecs::Entity _e) \
			{ \
				const auto& c = _coord.GetComponent<ComponentT>(_e); \
				return ecs_serial::Serialize<ComponentT>(c); \
			} \
		); \
	} while(0)