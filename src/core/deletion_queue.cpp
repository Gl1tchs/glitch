#include "core/deletion_queue.h"

void DeletionQueue::push_function(std::function<void()>&& p_function) {
	deletors.push_back(p_function);
}

void DeletionQueue::flush() {
	for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
		(*it)(); // call function
	}

	deletors.clear();
}
