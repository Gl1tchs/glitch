#pragma once

struct DeletionQueue {
	std::deque<std::function<void()>> deletors;
	std::vector<std::size_t> hashes;

	void push_function(std::function<void()>&& func);

	void push_hashed(std::size_t hash, std::function<void()>&& func);

	bool hash_exists(std::size_t hash) const;

	void flush();
};
