#include "glitch/scene/registry.h"

namespace gl {

Registry::~Registry() {
	// TODO: maybe dynamic destruction and resource management
}

EntityId Registry::spawn() {
	if (!free_indices.empty()) {
		uint32_t new_idx = free_indices.front();
		free_indices.pop();

		EntityId new_id = create_entity_id(
				new_idx, get_entity_version(entities[new_idx].id));

		entities[new_idx].id = new_id;

		return new_id;
	}

	entities.push_back(
			{ create_entity_id(entities.size(), 0), ComponentMask() });

	return entities.back().id;
}

bool Registry::is_valid(EntityId p_entity) {
	if (get_entity_index(p_entity) >= entities.size()) {
		return false;
	}

	return entities[get_entity_index(p_entity)].id == p_entity;
}

void Registry::despawn(EntityId p_entity) {
	const uint32_t entity_idx = get_entity_index(p_entity);

	EntityId new_entity_id =
			create_entity_id(UINT32_MAX, get_entity_version(p_entity) + 1);

	entities[entity_idx].id = new_entity_id;
	entities[entity_idx].mask.reset();

	free_indices.push(entity_idx);
}

} //namespace gl