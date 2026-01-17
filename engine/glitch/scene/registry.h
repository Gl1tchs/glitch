/**
 * @file registry.h
 */

#pragma once

#include "glitch/core/core.h"
#include "glitch/core/templates/concepts.h"
#include "glitch/scene/component_lookup.h"
#include "glitch/scene/view.h"

#include <queue>

namespace gl {

/**
 * Container of entities and components assigned to them.
 */
class GL_API Registry {
public:
	~Registry();

	void clear();

	void copy_to(Registry& dest);

	/**
	 * Create new entity instance on the scene
	 */
	EntityId spawn();

	/**
	 * Find out wether the entity is valid or not
	 */
	bool is_valid(EntityId entity);

	/**
	 * Removes entity from the scene and increments
	 * version
	 */
	void despawn(EntityId entity);

	/**
	 * Assigns specified component to the entity
	 */
	template <typename T, typename... TArgs> T* assign(EntityId entity, TArgs&&... args) {
		if (!is_valid(entity)) {
			return nullptr;
		}

		const uint32_t component_id = get_component_id<T>();

		if (_component_pools.size() <= component_id) {
			_component_pools.resize(component_id + 1, nullptr);
			_pool_helpers.resize(component_id + 1);
		}
		if (_component_pools[component_id] == nullptr) {
			_component_pools[component_id] = new ComponentPool(sizeof(T));
			_pool_helpers[component_id] = PoolHelpers{
				.element_size = sizeof(T),
				// Copy function (uses placement new + copy constructor)
				.copy_fn = [](void* dest,
								   const void* src) { new (dest) T(*static_cast<const T*>(src)); },
				// Destroy function (calls destructor)
				.destroy_fn = [](void* data) { static_cast<T*>(data)->~T(); },
			};
		}

		// Call destructor if component already exists
		if (_entities[get_entity_index(entity)].mask.test(component_id)) {
			_pool_helpers[component_id].destroy_fn(
					_component_pools[component_id]->get(get_entity_index(entity)));
		}

		T* component = new (_component_pools[component_id]->get(get_entity_index(entity)))
				T(std::forward<TArgs>(args)...);

		_entities[get_entity_index(entity)].mask.set(component_id);

		return component;
	}

	/**
	 * Assigns specified components to the entity
	 */
	template <typename... TComponents>
		requires MultiParameter<TComponents...>
	std::tuple<TComponents*...> assign(EntityId entity) {
		if (!is_valid(entity)) {
			return std::make_tuple(static_cast<TComponents*>(nullptr)...);
		}

		return std::make_tuple(assign<TComponents>(entity)...);
	}

	/**
	 * Remove specified component from the entity
	 */
	template <typename T> void remove(EntityId entity) {
		if (!is_valid(entity)) {
			return;
		}

		const uint32_t component_id = get_component_id<T>();
		const uint32_t entity_idx = get_entity_index(entity);

		if (_entities[entity_idx].mask.test(component_id)) {
			// call component's destructor
			if (_pool_helpers.size() > component_id && _pool_helpers[component_id].destroy_fn) {
				_pool_helpers[component_id].destroy_fn(
						_component_pools[component_id]->get(entity_idx));
			}
			_entities[entity_idx].mask.reset(component_id);
		}
	}

	/**
	 * Remove specified components from the entity
	 */
	template <typename... TComponents>
		requires MultiParameter<TComponents...>
	void remove(EntityId entity) {
		if (!is_valid(entity)) {
			return;
		}

		(remove<TComponents>(entity), ...);
	}

	/**
	 * Get specified component from the entity
	 */
	template <typename T> T* get(EntityId entity) {
		if (!is_valid(entity)) {
			return nullptr;
		}

		const uint32_t component_id = get_component_id<T>();
		if (!_entities[get_entity_index(entity)].mask.test(component_id)) {
			return nullptr;
		}

		T* component =
				static_cast<T*>(_component_pools[component_id]->get(get_entity_index(entity)));

		return component;
	}

	/**
	 * Get specified components from the entity
	 */
	template <typename... TComponents>
		requires MultiParameter<TComponents...>
	std::tuple<TComponents*...> get(EntityId entity) {
		if (!is_valid(entity)) {
			return std::make_tuple(static_cast<TComponents*>(nullptr)...);
		}

		return std::make_tuple(get<TComponents>(entity)...);
	}

	/**
	 * Find out wether an entity has the specified components
	 */
	template <typename... TComponents> bool has(EntityId entity) {
		if (!is_valid(entity)) {
			return false;
		}

		const uint32_t component_ids[] = { get_component_id<TComponents>()... };
		for (int i = 0; i < sizeof...(TComponents); i++) {
			if (!_entities[get_entity_index(entity)].mask.test(component_ids[i])) {
				return false;
			}
		}

		return true;
	}

	/**
	 * Get entities with specified components,
	 * if no component provided it will return all
	 * of the entities
	 */
	template <typename... TComponents> SceneView<TComponents...> view() {
		return SceneView<TComponents...>(&_entities);
	}

private:
	struct PoolHelpers {
		size_t element_size = 0;
		void (*copy_fn)(void*, const void*) = nullptr;
		void (*destroy_fn)(void*) = nullptr;
	};

	uint32_t _entity_counter = 0;
	EntityContainer _entities;
	std::queue<EntityId> _free_indices;
	std::vector<ComponentPool*> _component_pools;
	// parallel vector to component_pools for component destruction logic
	std::vector<PoolHelpers> _pool_helpers;
};

} //namespace gl