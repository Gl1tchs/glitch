/**
 * @file concepts.h
 */

#pragma once

namespace gl {

template <typename... Ts>
concept SingleParameter = sizeof...(Ts) == 1;

template <typename... Ts>
concept MultiParameter = sizeof...(Ts) > 1;

template <typename T, typename... U>
concept IsAnyOf = (std::same_as<T, U> || ...);

template <typename T>
concept Serializable = requires {
	{
		T::serialize(std::declval<std::string_view>(), std::declval<const T&>())
	} -> std::same_as<bool>;
	{ T::deserialize(std::declval<std::string_view>(), std::declval<T&>()) } -> std::same_as<bool>;
};

} //namespace gl