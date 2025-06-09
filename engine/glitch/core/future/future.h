/**
 * @file future.h
 *
 */

#pragma once

template <typename T> using Future = std::future<T>;

template <typename T> bool is_future_ready(const std::future<T>& p_fut) {
	return p_fut.valid() &&
			p_fut.wait_for(std::chrono::seconds(0)) ==
			std::future_status::ready;
}
