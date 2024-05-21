#include "gl/core/deletion_queue.h"

void DeletionQueue::push_function(std::function<void()>&& func) {
	deletors.push_back(func);
}

void DeletionQueue::flush() {
	for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
		(*it)(); // call function
	}

	deletors.clear();
}
