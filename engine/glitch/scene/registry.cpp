#include "glitch/scene/registry.h"

namespace gl {

Registry::~Registry() { clear(); }

void Registry::clear() {
	// call component's destructores
	for (size_t entity_idx = 0; entity_idx < entities.size(); ++entity_idx) {
		for (size_t comp_id = 0; comp_id < component_pools.size(); ++comp_id) {
			if (entities[entity_idx].mask.test(comp_id) &&
					pool_helpers[comp_id].destroy_fn) {
				// call the destructor
				pool_helpers[comp_id].destroy_fn(
						component_pools[comp_id]->get(entity_idx));
			}
		}
	}

	// delete the pools
	for (ComponentPool* pool : component_pools) {
		delete pool;
	}

	// Clear all data
	component_pools.clear();
	pool_helpers.clear();
	entities.clear();
	free_indices = {};
	entity_counter = 0;
}

void Registry::copy_to(Registry& p_dest) {
	p_dest.clear();

	// Copy trivial data
	p_dest.entity_counter = this->entity_counter;
	p_dest.free_indices = this->free_indices;
	p_dest.entities =
			this->entities; // This copies versions and component masks

	// Prepare destination pools
	p_dest.component_pools.resize(this->component_pools.size(), nullptr);
	p_dest.pool_helpers = this->pool_helpers;

	// Iterate all pools and copy component data
	for (size_t comp_id = 0; comp_id < this->component_pools.size();
			comp_id++) {
		if (this->component_pools[comp_id] == nullptr) {
			continue; // This component type isn't used
		}

		// Get copy function
		auto& helper = this->pool_helpers[comp_id];

		// Create a new, empty pool in the destination
		p_dest.component_pools[comp_id] =
				new ComponentPool(helper.element_size);

		// Iterate all entities and copy components
		for (size_t entity_idx = 0; entity_idx < this->entities.size();
				entity_idx++) {
			// If the entity has this component, copy it
			if (this->entities[entity_idx].mask.test(comp_id)) {
				void* dest_ptr =
						p_dest.component_pools[comp_id]->get(entity_idx);
				void* src_ptr = this->component_pools[comp_id]->get(entity_idx);

				helper.copy_fn(dest_ptr, src_ptr);
			}
		}
	}
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