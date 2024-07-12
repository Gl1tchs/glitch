#pragma once

inline constexpr uint32_t MAX_ENTITIES = 1000; // TODO: dynamically allocate
inline constexpr uint32_t MAX_COMPONENTS = 32;

// first 32 bits is index and last 32 bits are version
typedef uint64_t Entity;

typedef std::bitset<MAX_COMPONENTS> ComponentMask;

struct EntityDescriptor {
	Entity id;
	ComponentMask mask;
};
typedef std::vector<EntityDescriptor> EntityContainer;

constexpr inline Entity create_entity_id(uint32_t p_index, uint32_t p_version) {
	return ((Entity)p_index << 32) | p_version;
}

constexpr inline uint32_t get_entity_index(Entity p_entity) {
	return p_entity >> 32;
}

constexpr inline uint32_t get_entity_version(Entity p_entity) {
	// this conversation will loose the top 32 bits
	return (uint32_t)p_entity;
}

constexpr inline bool is_entity_valid(Entity p_entity) {
	return (p_entity >> 32) != -1;
}

inline constexpr Entity INVALID_ENTITY = create_entity_id(UINT32_MAX, 0);
