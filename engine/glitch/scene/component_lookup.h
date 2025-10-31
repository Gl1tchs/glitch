/**
 * @file component_lookup.h
 */

#pragma once

namespace gl {

inline constexpr uint32_t MAX_ENTITIES = 1000; // TODO: dynamically allocate
inline constexpr uint32_t MAX_COMPONENTS = 32;

// first 32 bits is index and last 32 bits are version
typedef uint64_t EntityId;

typedef std::bitset<MAX_COMPONENTS> ComponentMask;

struct EntityDescriptor {
	EntityId id;
	ComponentMask mask;
};
typedef std::vector<EntityDescriptor> EntityContainer;

constexpr inline EntityId create_entity_id(
		uint32_t p_index, uint32_t p_version) {
	return ((EntityId)p_index << 32) | p_version;
}

constexpr inline uint32_t get_entity_index(EntityId p_entity) {
	return p_entity >> 32;
}

constexpr inline uint32_t get_entity_version(EntityId p_entity) {
	// this conversation will loose the top 32 bits
	return (uint32_t)p_entity;
}

constexpr inline bool is_entity_valid(EntityId p_entity) {
	return (p_entity >> 32) != -1;
}

inline constexpr EntityId INVALID_ENTITY_ID = create_entity_id(UINT32_MAX, 0);

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

} //namespace gl