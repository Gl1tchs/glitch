#pragma once

#include "scene/entity.h"

inline uint32_t s_component_counter = 0;

// returns different id for different component types
template <class T> inline uint32_t get_component_id() {
	static uint32_t s_component_id = s_component_counter++;
	return s_component_id;
}

class ComponentPool {
public:
	ComponentPool(size_t p_element_size) : element_size(p_element_size) {
		data = new uint8_t[element_size * MAX_ENTITIES];
	}

	~ComponentPool() { delete[] data; }

	void* get(size_t p_index) { return data + p_index * element_size; }

private:
	uint8_t* data = nullptr;
	size_t element_size = 0;
};
