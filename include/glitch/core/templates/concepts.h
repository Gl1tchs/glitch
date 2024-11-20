#pragma once

template <typename... Ts>
concept SingleParameter = sizeof...(Ts) == 1;

template <typename... Ts>
concept MultiParameter = sizeof...(Ts) > 1;
