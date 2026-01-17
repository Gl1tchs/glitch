#pragma once

#include <deque>
#include <functional>

namespace gl {

/**
 * Struct representing a queue which deletor functions can be assigned into and
 * later they will be deleted in the reverse order you have pushed.
 */
struct DeletionQueue {
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) { deletors.push_back(function); }

	void flush() {
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); // call function
		}

		deletors.clear();
	}
};

} //namespace gl