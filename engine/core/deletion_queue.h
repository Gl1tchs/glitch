#pragma once

struct DeletionQueue {
	std::deque<std::function<void()>> deletors;

	inline void push_function(std::function<void()>&& func) {
		deletors.push_back(func);
	}

	inline void flush() {
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); // call function
		}

		deletors.clear();
	}
};
