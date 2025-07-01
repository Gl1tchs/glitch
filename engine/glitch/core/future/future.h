/**
 * @file future.h
 *
 */

#pragma once

template <typename T> class Future {
public:
	Future() = default;
	Future(std::future<T>&& p_fut) : fut(std::move(p_fut)) {}

	template <typename F> static Future async(F&& func) {
		return Future(std::async(std::launch::async, std::forward<F>(func)));
	}

	bool is_ready() const {
		return fut.valid() &&
				fut.wait_for(std::chrono::seconds(0)) ==
				std::future_status::ready;
	}

	Optional<T> get() {
		if (fut.valid()) {
			return fut.get();
		}

		return {};
	}

private:
	std::future<T> fut;
};