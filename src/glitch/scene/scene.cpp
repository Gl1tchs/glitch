#include "glitch/scene/scene.h"

Entity Scene::create() {
	if (!free_indices.empty()) {
		uint32_t new_idx = free_indices.front();
		free_indices.pop();

		Entity new_id = create_entity_id(
				new_idx, get_entity_version(entities[new_idx].id));

		entities[new_idx].id = new_id;

		return new_id;
	}

	entities.push_back(
			{ create_entity_id(entities.size(), 0), ComponentMask() });

	return entities.back().id;
}

bool Scene::is_valid(Entity p_entity) {
	if (get_entity_index(p_entity) >= entities.size()) {
		return false;
	}

	return entities[get_entity_index(p_entity)].id == p_entity;
}

void Scene::destroy(Entity p_entity) {
	const uint32_t entity_idx = get_entity_index(p_entity);

	Entity new_entity_id =
			create_entity_id(UINT32_MAX, get_entity_version(p_entity) + 1);

	entities[entity_idx].id = new_entity_id;
	entities[entity_idx].mask.reset();

	free_indices.push(entity_idx);
}
