/**
 * @file concepts.h
 */

#pragma once

namespace gl {

template <typename... Ts>
concept SingleParameter = sizeof...(Ts) == 1;

template <typename... Ts>
concept MultiParameter = sizeof...(Ts) > 1;

} //namespace gl