/**
 * @file future.h
 *
 */

#pragma once

namespace gl {

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

	Result<T, bool> get() {
		if (fut.valid()) {
			return fut.get();
		}

		return make_err<T>(false);
	}

private:
	std::future<T> fut;
};

} //namespace gl