#include "glitch/scene/component_lookup.h"

#include <cstring>

namespace gl {

ComponentPool::ComponentPool(size_t element_size) : _element_size(element_size) {}

ComponentPool::~ComponentPool() {}

ComponentPool::ComponentPool(const ComponentPool& other) : _element_size(other._element_size) {
	for (const auto& page : other._pages) {
		if (!page) {
			continue;
		}

		auto new_page = std::make_unique<uint8_t[]>(PAGE_SIZE * _element_size);
		std::memcpy(new_page.get(), page.get(), PAGE_SIZE * _element_size);
		_pages.push_back(std::move(new_page));
	}
}

size_t ComponentPool::get_size() const { return _element_size; }

void* ComponentPool::get(size_t idx) {
	const size_t page_idx = idx / PAGE_SIZE;
	const size_t offset = idx % PAGE_SIZE;

	if (page_idx >= _pages.size()) {
		_pages.resize(page_idx + 1);
	}

	if (!_pages[page_idx]) {
		_pages[page_idx] = std::make_unique<uint8_t[]>(PAGE_SIZE * _element_size);
	}

	return _pages[page_idx].get() + (offset * _element_size);
}

} //namespace gl