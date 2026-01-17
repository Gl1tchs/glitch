/**
 * @file paged_allocator.h
 */

#pragma once

#include "glitch/core/memory/memory.h"

#include <cstddef>
#include <vector>

namespace gl {

/**
 * Class representing a dynamic allocator that will grow when
 * the underlying data exceeds `page_size`
 */
template <typename T> class PagedAllocator {
public:
	PagedAllocator(size_t page_size = 4096) : _page_size(page_size) { _allocate_new_page(); }

	~PagedAllocator() {
		for (auto page : _pages) {
			GL_FREE(page);
		}
	}

	T* alloc() {
		if (_free_list.empty()) {
			_allocate_new_page();
		}
		T* obj = _free_list.back();
		_free_list.pop_back();
		return obj;
	}

	void free(T* obj) { _free_list.push_back(obj); }

private:
	size_t _page_size;
	std::vector<T*> _pages;
	std::vector<T*> _free_list;

	void _allocate_new_page() {
		T* page = GL_ALLOC_ARRAY(T, _page_size);
		_pages.push_back(page);
		for (size_t i = 0; i < _page_size; ++i) {
			_free_list.push_back(&page[i]);
		}
	}
};

} //namespace gl