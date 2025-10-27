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

} //namespace gl