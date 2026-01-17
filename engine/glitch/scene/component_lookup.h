/**
 * @file component_lookup.h
 */

#pragma once

#include <bitset>
#include <cstdint>
#include <memory>
#include <vector>

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

constexpr inline EntityId create_entity_id(uint32_t p_index, uint32_t p_version) {
	return ((EntityId)p_index << 32) | p_version;
}

constexpr inline uint32_t get_entity_index(EntityId p_entity) { return p_entity >> 32; }

constexpr inline uint32_t get_entity_version(EntityId p_entity) {
	// this conversation will loose the top 32 bits
	return (uint32_t)p_entity;
}

constexpr inline bool is_entity_valid(EntityId p_entity) { return (p_entity >> 32) != UINT32_MAX; }

inline constexpr EntityId INVALID_ENTITY_ID = create_entity_id(UINT32_MAX, 0);

inline uint32_t s_component_counter = 0;

// returns different id for different component types
template <class T> inline uint32_t get_component_id() {
	static uint32_t s_component_id = s_component_counter++;
	return s_component_id;
}

class ComponentPool {
public:
	static constexpr size_t PAGE_SIZE = 1024;

	ComponentPool(size_t element_size);
	~ComponentPool();

	ComponentPool(const ComponentPool& other);

	size_t get_size() const;

	void* get(size_t idx);

	template <std::default_initializable T> T* add(uint32_t idx);

private:
	std::vector<std::unique_ptr<uint8_t[]>> _pages;
	size_t _element_size = 0;
};

} //namespace gl