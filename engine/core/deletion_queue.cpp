#include "core/deletion_queue.h"

void DeletionQueue::push_function(std::function<void()>&& func) {
	deletors.push_back(func);
}

void DeletionQueue::push_hashed(
		std::size_t hash, std::function<void()>&& func) {
	hashes.push_back(hash);
	deletors.push_back(func);
}

bool DeletionQueue::hash_exists(std::size_t hash) const {
	auto it = std::find(hashes.begin(), hashes.end(), hash);
	return it != hashes.end();
}

void DeletionQueue::flush() {
	for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
		(*it)(); // call function
	}

	deletors.clear();
}
