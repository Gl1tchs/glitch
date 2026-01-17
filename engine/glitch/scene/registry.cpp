#include "glitch/scene/registry.h"

namespace gl {

Registry::~Registry() { clear(); }

void Registry::clear() {
	// call component's destructores
	for (size_t entity_idx = 0; entity_idx < _entities.size(); ++entity_idx) {
		for (size_t comp_id = 0; comp_id < _component_pools.size(); ++comp_id) {
			if (_entities[entity_idx].mask.test(comp_id) && _pool_helpers[comp_id].destroy_fn) {
				// call the destructor
				_pool_helpers[comp_id].destroy_fn(_component_pools[comp_id]->get(entity_idx));
			}
		}
	}

	// delete the pools
	for (ComponentPool* pool : _component_pools) {
		delete pool;
	}

	// Clear all data
	_component_pools.clear();
	_pool_helpers.clear();
	_entities.clear();
	_free_indices = {};
	_entity_counter = 0;
}

void Registry::copy_to(Registry& dest) {
	dest.clear();

	// Copy trivial data
	dest._entity_counter = this->_entity_counter;
	dest._free_indices = this->_free_indices;
	dest._entities = this->_entities; // This copies versions and component masks

	// Prepare destination pools
	dest._component_pools.resize(this->_component_pools.size(), nullptr);
	dest._pool_helpers = this->_pool_helpers;

	// Iterate all pools and copy component data
	for (size_t comp_id = 0; comp_id < this->_component_pools.size(); comp_id++) {
		if (this->_component_pools[comp_id] == nullptr) {
			continue; // This component type isn't used
		}

		// Get copy function
		auto& helper = this->_pool_helpers[comp_id];

		// Create a new, empty pool in the destination
		dest._component_pools[comp_id] = new ComponentPool(helper.element_size);

		// Iterate all entities and copy components
		for (size_t entity_idx = 0; entity_idx < this->_entities.size(); entity_idx++) {
			// If the entity has this component, copy it
			if (this->_entities[entity_idx].mask.test(comp_id)) {
				void* dest_ptr = dest._component_pools[comp_id]->get(entity_idx);
				void* src_ptr = this->_component_pools[comp_id]->get(entity_idx);

				helper.copy_fn(dest_ptr, src_ptr);
			}
		}
	}
}

EntityId Registry::spawn() {
	if (!_free_indices.empty()) {
		uint32_t new_idx = _free_indices.front();
		_free_indices.pop();

		EntityId new_id = create_entity_id(new_idx, get_entity_version(_entities[new_idx].id));

		_entities[new_idx].id = new_id;

		return new_id;
	}

	_entities.push_back({ create_entity_id(_entities.size(), 0), ComponentMask() });

	return _entities.back().id;
}

bool Registry::is_valid(EntityId entity) {
	if (get_entity_index(entity) >= _entities.size()) {
		return false;
	}

	return _entities[get_entity_index(entity)].id == entity;
}

void Registry::despawn(EntityId entity) {
	const uint32_t entity_idx = get_entity_index(entity);

	// Destroy all components
	for (size_t comp_id = 0; comp_id < _component_pools.size(); ++comp_id) {
		if (_entities[entity_idx].mask.test(comp_id)) {
			if (_pool_helpers.size() > comp_id && _pool_helpers[comp_id].destroy_fn) {
				_pool_helpers[comp_id].destroy_fn(_component_pools[comp_id]->get(entity_idx));
			}
		}
	}

	EntityId new_entity_id = create_entity_id(UINT32_MAX, get_entity_version(entity) + 1);

	_entities[entity_idx].id = new_entity_id;
	_entities[entity_idx].mask.reset();

	_free_indices.push(entity_idx);
}

} //namespace gl