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

/**
 * Concept representing a type that is either serializable to file or json
 */
template <typename T>
concept IsSerializable = requires {
	{ T::serialize(std::declval<json&>(), std::declval<const T&>()) } -> std::same_as<bool>;
	{ T::deserialize(std::declval<const json&>(), std::declval<T&>()) } -> std::same_as<bool>;
};

} //namespace gl
