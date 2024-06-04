#pragma once

template <typename T> class PagedAllocator {
public:
	PagedAllocator(size_t p_page_size = 4096) : page_size(p_page_size) {
		_allocate_new_page();
	}

	~PagedAllocator() {
		for (auto page : pages) {
			GL_FREE(page);
		}
	}

	T* alloc() {
		if (free_list.empty()) {
			_allocate_new_page();
		}
		T* obj = free_list.back();
		free_list.pop_back();
		return obj;
	}

	void free(T* obj) { free_list.push_back(obj); }

private:
	size_t page_size;
	std::vector<T*> pages;
	std::vector<T*> free_list;

	void _allocate_new_page() {
		T* page = GL_ALLOC_ARRAY(T, page_size);
		pages.push_back(page);
		for (size_t i = 0; i < page_size; ++i) {
			free_list.push_back(&page[i]);
		}
	}
};
